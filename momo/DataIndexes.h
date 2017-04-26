/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/DataIndexes.h

\**********************************************************/

#pragma once

#include "HashMultiMap.h"

namespace momo
{

namespace experimental
{

namespace internal
{
	struct DataHashSetSettings : public momo::HashSetSettings
	{
		static const CheckMode checkMode = CheckMode::assertion;
		static const ExtraCheckMode extraCheckMode = ExtraCheckMode::nothing;
		static const bool checkVersion = false;
	};

	struct DataHashMapSettings : public momo::HashMapSettings
	{
		static const CheckMode checkMode = CheckMode::assertion;
		static const ExtraCheckMode extraCheckMode = ExtraCheckMode::nothing;
		static const bool checkVersion = false;
	};

	struct DataHashMultiMapSettings : public momo::HashMultiMapSettings
	{
		static const CheckMode checkMode = CheckMode::assertion;
		static const ExtraCheckMode extraCheckMode = ExtraCheckMode::nothing;
		static const bool checkKeyVersion = false;
		static const bool checkValueVersion = false;
	};

	template<typename TColumnList, typename TDataTraits>
	class DataIndexes
	{
	public:
		typedef TColumnList ColumnList;
		typedef TDataTraits DataTraits;
		typedef typename ColumnList::MemManager MemManager;
		typedef typename ColumnList::Settings Settings;
		typedef typename ColumnList::Raw Raw;

		template<typename Type>
		using Column = typename ColumnList::template Column<Type>;

		template<typename... Types>
		using OffsetItemTuple = std::tuple<std::pair<size_t, const Types&>...>;

	private:
		typedef momo::internal::MemManagerPtr<MemManager> MemManagerPtr;

		template<typename Item, size_t internalCapacity = 0>
		using Array = momo::Array<Item, MemManagerPtr, ArrayItemTraits<Item, MemManagerPtr>,
			momo::internal::NestedArraySettings<ArraySettings<internalCapacity>>>;

		typedef Array<size_t> Offsets;

		typedef std::function<size_t(Raw*, size_t*)> HashFunc;
		typedef std::function<bool(Raw*, Raw*)> EqualFunc;

		struct HashRawKey
		{
			Raw* raw;
			size_t hashCode;
		};

		template<typename... Types>
		struct HashTupleKey
		{
			OffsetItemTuple<Types...> tuple;
			size_t hashCode;
			const ColumnList* columnList;
		};

		class HashBucketStater
		{
		public:
			static unsigned char GetState(const HashRawKey* key) MOMO_NOEXCEPT
			{
				return (key->raw != nullptr) ? (unsigned char)1 : (unsigned char)key->hashCode;
			}

			template<typename Item>
			static unsigned char GetState(const Item* item) MOMO_NOEXCEPT
			{
				return GetState(item->GetKeyPtr());
			}

			static void SetState(HashRawKey* key, unsigned char state) MOMO_NOEXCEPT
			{
				MOMO_ASSERT(state != (unsigned char)1 || key->raw != nullptr);
				if (state != (unsigned char)1)
				{
					key->raw = nullptr;
					key->hashCode = (size_t)state;
				}
			}

			template<typename Item>
			static void SetState(Item* item, unsigned char state) MOMO_NOEXCEPT
			{
				return SetState(item->GetKeyPtr(), state);
			}
		};

		class HashTraits : public momo::HashTraits<HashRawKey, HashBucketOneI<HashBucketStater>>
		{
		public:
			template<typename KeyArg>
			struct IsValidKeyArg : std::false_type
			{
			};

			template<typename... Types>
			struct IsValidKeyArg<HashTupleKey<Types...>> : public std::true_type
			{
			};

		public:
			explicit HashTraits(EqualFunc&& equalFunc)
				: mEqualFunc(std::move(equalFunc))
			{
			}

			template<typename HashKey>
			size_t GetHashCode(const HashKey& key) const MOMO_NOEXCEPT
			{
				return key.hashCode;
			}

			bool IsEqual(const HashRawKey& key1, const HashRawKey& key2) const
			{
				return key1.hashCode == key2.hashCode && mEqualFunc(key1.raw, key2.raw);
			}

			template<typename... Types>
			bool IsEqual(const HashTupleKey<Types...>& key1, const HashRawKey& key2) const
			{
				return key1.hashCode == key2.hashCode && pvIsEqual<0>(key1, key2);
			}

		private:
			template<size_t number, typename... Types>
			bool pvIsEqual(const HashTupleKey<Types...>& key1, const HashRawKey& key2,
				typename std::enable_if<(number < sizeof...(Types)), int>::type = 0) const
			{
				const auto& pair = std::get<number>(key1.tuple);
				const auto& item1 = pair.second;
				typedef typename std::decay<decltype(item1)>::type Type;
				const Type& item2 = key1.columnList->template GetByOffset<const Type>(key2.raw, pair.first);
				return DataTraits::IsEqual(item1, item2) && pvIsEqual<number + 1>(key1, key2);
			}

			template<size_t number, typename... Types>
			bool pvIsEqual(const HashTupleKey<Types...>& /*key1*/, const HashRawKey& /*key2*/,
				typename std::enable_if<(number == sizeof...(Types)), int>::type = 0) const MOMO_NOEXCEPT
			{
				return true;
			}

		private:
			EqualFunc mEqualFunc;
		};

	public:
		class UniqueHash;

		class UniqueIndexViolation : public std::runtime_error
		{
		public:
			UniqueIndexViolation(Raw* raw, const UniqueHash& uniqueHash)
				: std::runtime_error("Unique index violation"),
				raw(raw),
				uniqueHash(uniqueHash)
			{
			}

		public:
			Raw* raw;
			const UniqueHash& uniqueHash;
		};

		class UniqueHash
		{
		private:
			typedef momo::HashSet<HashRawKey, HashTraits, MemManagerPtr,
				HashSetItemTraits<HashRawKey, HashRawKey, MemManagerPtr>, DataHashSetSettings> HashSet;

		public:
			typedef typename HashSet::ConstIterator Iterator;

			class RawBounds
			{
			public:
				typedef Raw* const* Iterator;

			public:
				explicit RawBounds(Raw* raw = nullptr) MOMO_NOEXCEPT
				{
					*mRaws = raw;
				}

				Iterator GetBegin() const MOMO_NOEXCEPT
				{
					return mRaws;
				}

				Iterator GetEnd() const MOMO_NOEXCEPT
				{
					return mRaws + GetCount();
				}

				MOMO_FRIENDS_BEGIN_END(const RawBounds&, Iterator)

				size_t GetCount() const MOMO_NOEXCEPT
				{
					return (*mRaws != nullptr) ? 1 : 0;
				}

			private:
				Raw* mRaws[1];	//?
			};

		public:
			UniqueHash(Offsets&& sortedOffsets, HashFunc&& hashFunc, EqualFunc&& equalFunc)
				: mSortedOffsets(std::move(sortedOffsets)),
				mHashFunc(std::move(hashFunc)),
				mHashSet(typename HashSet::HashTraits(std::move(equalFunc)),
					MemManagerPtr(mSortedOffsets.GetMemManager()))
			{
			}

			UniqueHash(UniqueHash&& uniqueHash) MOMO_NOEXCEPT
				: mSortedOffsets(std::move(uniqueHash.mSortedOffsets)),
				mHashFunc(std::move(uniqueHash.mHashFunc)),
				mHashSet(std::move(uniqueHash.mHashSet))
			{
			}

			UniqueHash(const UniqueHash&) = delete;

			~UniqueHash() MOMO_NOEXCEPT
			{
			}

			UniqueHash& operator=(UniqueHash&& uniqueHash) MOMO_NOEXCEPT
			{
				mSortedOffsets = std::move(uniqueHash.mSortedOffsets);
				mHashFunc = std::move(uniqueHash.mHashFunc);
				mHashSet = std::move(uniqueHash.mHashSet);
				return *this;
			}

			UniqueHash& operator=(const UniqueHash&) = delete;

			const Offsets& GetSortedOffsets() const MOMO_NOEXCEPT
			{
				return mSortedOffsets;
			}
			
			void Reserve(size_t capacity)
			{
				mHashSet.Reserve(capacity);
			}

			Iterator Find(Raw* raw, size_t* hashCodes)
			{
				size_t hashCode = mHashFunc(raw, hashCodes);
				Iterator iter = mHashSet.Find({ raw, hashCode });
				MOMO_ASSERT(!!iter);
				return iter;
			}

			RawBounds Find(Raw* raw) const
			{
				size_t hashCode = mHashFunc(raw, nullptr);
				Iterator iter = mHashSet.Find({ raw, hashCode });
				return RawBounds(!!iter ? iter->raw : nullptr);
			}

			template<typename... Types>
			RawBounds Find(const HashTupleKey<Types...>& hashTupleKey) const
			{
				Iterator iter = mHashSet.Find(hashTupleKey);
				return RawBounds(!!iter ? iter->raw : nullptr);
			}

			void Clear() MOMO_NOEXCEPT
			{
				mHashSet.Clear();
			}

			Iterator Add(Raw* raw, size_t* hashCodes)
			{
				size_t hashCode = mHashFunc(raw, hashCodes);
				auto insRes = mHashSet.Insert({ raw, hashCode });
				if (!insRes.inserted)
					throw UniqueIndexViolation(insRes.iterator->raw, *this);
				return insRes.iterator;
			}

			Iterator Insert(Raw* raw, size_t* hashCodes)
			{
				size_t hashCode = mHashFunc(raw, hashCodes);
				return mHashSet.Insert({ raw, hashCode }).iterator;
			}

			void Remove(Iterator iter) MOMO_NOEXCEPT
			{
				mHashSet.Remove(iter);
			}

		private:
			Offsets mSortedOffsets;
			HashFunc mHashFunc;
			HashSet mHashSet;
		};

		class MultiHash
		{
		private:
			typedef momo::HashMultiMap<HashRawKey, Raw*, HashTraits, MemManagerPtr,
				HashMultiMapKeyValueTraits<HashRawKey, Raw*, MemManagerPtr>,
				DataHashMultiMapSettings> HashMultiMap;

			typedef momo::HashMap<Raw*, size_t, momo::HashTraits<Raw*>, MemManagerPtr,
				HashMapKeyValueTraits<Raw*, size_t, MemManagerPtr>, DataHashMapSettings> HashMap;	//?

			static const size_t rawFastCount = 8;

		public:
			typedef typename HashMultiMap::Iterator Iterator;

			typedef typename HashMultiMap::ConstValueBounds RawBounds;

		public:
			MultiHash(Offsets&& sortedOffsets, HashFunc&& hashFunc, EqualFunc&& equalFunc)
				: mSortedOffsets(std::move(sortedOffsets)),
				mHashFunc(std::move(hashFunc)),
				mHashMultiMap(typename HashMultiMap::HashTraits(std::move(equalFunc)),
					MemManagerPtr(mSortedOffsets.GetMemManager())),
				mHashMap(typename HashMap::HashTraits(), MemManagerPtr(mSortedOffsets.GetMemManager()))
			{
			}

			MultiHash(MultiHash&& multiHash) MOMO_NOEXCEPT
				: mSortedOffsets(std::move(multiHash.mSortedOffsets)),
				mHashFunc(std::move(multiHash.mHashFunc)),
				mHashMultiMap(std::move(multiHash.mHashMultiMap)),
				mHashMap(std::move(multiHash.mHashMap))
			{
			}

			MultiHash(const MultiHash&) = delete;

			~MultiHash() MOMO_NOEXCEPT
			{
			}

			MultiHash& operator=(MultiHash&& multiHash) MOMO_NOEXCEPT
			{
				mSortedOffsets = std::move(multiHash.mSortedOffsets);
				mHashFunc = std::move(multiHash.mHashFunc);
				mHashMultiMap = std::move(multiHash.mHashMultiMap);
				mHashMap = std::move(multiHash.mHashMap);
				return *this;
			}

			MultiHash& operator=(const MultiHash&) = delete;

			const Offsets& GetSortedOffsets() const MOMO_NOEXCEPT
			{
				return mSortedOffsets;
			}

			size_t GetKeyCount() const MOMO_NOEXCEPT
			{
				return mHashMultiMap.GetKeyCount();
			}

			Iterator Find(Raw* raw, size_t* hashCodes)
			{
				size_t hashCode = mHashFunc(raw, hashCodes);
				auto keyIter = mHashMultiMap.Find({ raw, hashCode });
				MOMO_ASSERT(!!keyIter);
				auto raws = keyIter->values;
				if (raws.GetCount() > rawFastCount)
				{
					auto mapIter = mHashMap.Find(raw);
					if (!!mapIter)
						return mHashMultiMap.MakeIterator(keyIter, mapIter->value);
				}
				return mHashMultiMap.MakeIterator(keyIter,
					std::find(raws.GetBegin(), raws.GetEnd(), raw) - raws.GetBegin());
			}

			template<typename... Types>
			RawBounds Find(const HashTupleKey<Types...>& hashTupleKey) const
			{
				auto keyIter = mHashMultiMap.Find(hashTupleKey);
				return !!keyIter ? keyIter->values : RawBounds();
			}

			void Clear() MOMO_NOEXCEPT
			{
				mHashMultiMap.Clear();
				mHashMap.Clear();
			}

			Iterator Add(Raw* raw, size_t* hashCodes)
			{
				size_t hashCode = mHashFunc(raw, hashCodes);
				Iterator iter = mHashMultiMap.Add({ raw, hashCode }, raw);
				auto raws = iter.GetKeyIterator()->values;
				if (raws.GetCount() > rawFastCount)
				{
					try
					{
						mHashMap.Insert(raw, iter.GetValuePtr() - raws.GetBegin());
					}
					catch (...)
					{
						mHashMultiMap.Remove(iter);
						throw;
					}
				}
				return iter;
			}

			void Remove(Iterator iter) MOMO_NOEXCEPT
			{
				Raw* raw = iter->value;
				auto keyIter = iter.GetKeyIterator();
				auto raws = keyIter->values;
				Raw* lastRaw = *(raws.GetEnd() - 1);
				if (raw != lastRaw)
				{
					auto mapIter = mHashMap.Find(lastRaw);
					if (!!mapIter)
						mapIter->value = iter.GetValuePtr() - raws.GetBegin();
				}
				mHashMultiMap.Remove(iter);
				mHashMap.Remove(raw);
				if (keyIter->key.raw == raw)
				{
					raws = keyIter->values;
					if (raws.GetCount() > 0)
						mHashMultiMap.ResetKey(keyIter, { raws[0], keyIter->key.hashCode });
					else
						mHashMultiMap.RemoveKey(keyIter);
				}
			}

		private:
			Offsets mSortedOffsets;
			HashFunc mHashFunc;
			HashMultiMap mHashMultiMap;
			HashMap mHashMap;
		};

	private:
		typedef Array<UniqueHash> UniqueHashes;
		typedef Array<typename UniqueHash::Iterator,
			Settings::indexIteratorsInternalCapacity> UniqueHashIterators;

		typedef Array<MultiHash> MultiHashes;
		typedef Array<typename MultiHash::Iterator,
			Settings::indexIteratorsInternalCapacity> MultiHashIterators;

		typedef Array<size_t> OffsetHashCodes;

	public:
		DataIndexes(const ColumnList* columnList, MemManager& memManager)
			: mColumnList(columnList),
			mUniqueHashes(MemManagerPtr(memManager)),
			mMultiHashes(MemManagerPtr(memManager)),
			mOffsetHashCodes(columnList->GetTotalSize(), (size_t)0, MemManagerPtr(memManager))
		{
		}

		DataIndexes(DataIndexes&& indexes) MOMO_NOEXCEPT
			: mColumnList(indexes.mColumnList),
			mUniqueHashes(std::move(indexes.mUniqueHashes)),
			mMultiHashes(std::move(indexes.mMultiHashes)),
			mOffsetHashCodes(std::move(indexes.mOffsetHashCodes))
		{
		}

		DataIndexes(const DataIndexes&) = delete;

		~DataIndexes() MOMO_NOEXCEPT
		{
		}

		DataIndexes& operator=(const DataIndexes&) = delete;

		void Swap(DataIndexes& indexes) MOMO_NOEXCEPT
		{
			std::swap(mColumnList, indexes.mColumnList);
			mUniqueHashes.Swap(indexes.mUniqueHashes);
			mMultiHashes.Swap(indexes.mMultiHashes);
			mOffsetHashCodes.Swap(indexes.mOffsetHashCodes);
		}

		template<typename... Types>
		const UniqueHash* GetUniqueHash(const Column<Types>&... columns) const
		{
			return pvGetHash(mUniqueHashes, columns...);
		}

		template<typename... Types>
		const MultiHash* GetMultiHash(const Column<Types>&... columns) const
		{
			return pvGetHash(mMultiHashes, columns...);
		}

		template<typename Raws, typename... Types>
		const UniqueHash* AddUniqueHash(const Raws& raws, const Column<Types>&... columns)
		{
			return pvAddHash(mUniqueHashes, raws, columns...);
		}

		template<typename Raws, typename... Types>
		const MultiHash* AddMultiHash(const Raws& raws, const Column<Types>&... columns)
		{
			return pvAddHash(mMultiHashes, raws, columns...);
		}

		template<typename... Types>
		bool RemoveUniqueHash(const Column<Types>&... columns)
		{
			return pvRemoveHash(mUniqueHashes, columns...);
		}

		template<typename... Types>
		bool RemoveMultiHash(const Column<Types>&... columns)
		{
			return pvRemoveHash(mMultiHashes, columns...);
		}

		typename UniqueHash::RawBounds FindRaws(const UniqueHash& uniqueHash, Raw* raw) const
		{
			return uniqueHash.Find(raw);	//?
		}

		template<typename Hash, typename... Types>
		typename Hash::RawBounds FindRaws(const Hash& hash, const OffsetItemTuple<Types...>& tuple) const
		{
			HashTupleKey<Types...> hashTupleKey{ tuple, pvGetHashCode<0>(tuple), mColumnList };
			return hash.Find(hashTupleKey);
		}

		void ClearRaws() MOMO_NOEXCEPT
		{
			for (UniqueHash& uniqueHash : mUniqueHashes)
				uniqueHash.Clear();
			for (MultiHash& multiHash : mMultiHashes)
				multiHash.Clear();
		}

		void Reserve(size_t capacity)
		{
			for (UniqueHash& uniqueHash : mUniqueHashes)
				uniqueHash.Reserve(capacity);
		}

		void AddRaw(Raw* raw)
		{
			size_t uniqueHashCount = mUniqueHashes.GetCount();
			UniqueHashIterators uniqueHashIters(uniqueHashCount, pvGetMemManagerPtr());
			size_t uniqueHashIndex = 0;
			size_t multiHashCount = mMultiHashes.GetCount();
			MultiHashIterators multiHashIters(multiHashCount, pvGetMemManagerPtr());
			size_t multiHashIndex = 0;
			size_t* hashCodes = nullptr;
			try
			{
				for (; uniqueHashIndex < uniqueHashCount; ++uniqueHashIndex)
					uniqueHashIters[uniqueHashIndex] = mUniqueHashes[uniqueHashIndex].Add(raw, hashCodes);
				for (; multiHashIndex < multiHashCount; ++multiHashIndex)
					multiHashIters[multiHashIndex] = mMultiHashes[multiHashIndex].Add(raw, hashCodes);
			}
			catch (...)
			{
				for (size_t i = 0; i < uniqueHashIndex; ++i)
					mUniqueHashes[i].Remove(uniqueHashIters[i]);
				for (size_t i = 0; i < multiHashIndex; ++i)
					mMultiHashes[i].Remove(multiHashIters[i]);
				throw;
			}
		}

		void RemoveRaw(Raw* raw)
		{
			size_t uniqueHashCount = mUniqueHashes.GetCount();
			UniqueHashIterators uniqueHashIters(uniqueHashCount, pvGetMemManagerPtr());
			size_t multiHashCount = mMultiHashes.GetCount();
			MultiHashIterators multiHashIters(multiHashCount, pvGetMemManagerPtr());
			size_t* hashCodes = nullptr;
			for (size_t i = 0; i < uniqueHashCount; ++i)
				uniqueHashIters[i] = mUniqueHashes[i].Find(raw, hashCodes);
			for (size_t i = 0; i < multiHashCount; ++i)
				multiHashIters[i] = mMultiHashes[i].Find(raw, hashCodes);
			for (size_t i = 0; i < uniqueHashCount; ++i)
				mUniqueHashes[i].Remove(uniqueHashIters[i]);
			for (size_t i = 0; i < multiHashCount; ++i)
				mMultiHashes[i].Remove(multiHashIters[i]);
		}

		void UpdateRaw(Raw* oldRaw, Raw* newRaw)
		{
			size_t uniqueHashCount = mUniqueHashes.GetCount();
			UniqueHashIterators oldUniqueHashIters(uniqueHashCount, pvGetMemManagerPtr());
			UniqueHashIterators newUniqueHashIters(uniqueHashCount, pvGetMemManagerPtr());
			size_t uniqueHashIndex = 0;
			size_t multiHashCount = mMultiHashes.GetCount();
			MultiHashIterators oldMultiHashIters(multiHashCount, pvGetMemManagerPtr());
			MultiHashIterators newMultiHashIters(multiHashCount, pvGetMemManagerPtr());
			size_t multiHashIndex = 0;
			try
			{
				for (; uniqueHashIndex < uniqueHashCount; ++uniqueHashIndex)
					newUniqueHashIters[uniqueHashIndex] = mUniqueHashes[uniqueHashIndex].Insert(newRaw, nullptr);
				for (size_t i = 0; i < uniqueHashCount; ++i)
				{
					auto newIter = newUniqueHashIters[i];
					if (newIter->raw != oldRaw)
					{
						if (newIter->raw != newRaw)
							throw UniqueIndexViolation(newIter->raw, mUniqueHashes[i]);
						oldUniqueHashIters[i] = mUniqueHashes[i].Find(oldRaw, nullptr);
					}
				}
				for (; multiHashIndex < multiHashCount; ++multiHashIndex)
					newMultiHashIters[multiHashIndex] = mMultiHashes[multiHashIndex].Add(newRaw, nullptr);
				for (size_t i = 0; i < multiHashCount; ++i)
					oldMultiHashIters[i] = mMultiHashes[i].Find(oldRaw, nullptr);
			}
			catch (...)
			{
				for (size_t i = 0; i < uniqueHashIndex; ++i)
				{
					auto newIter = newUniqueHashIters[i];
					if (newIter->raw == newRaw)
						mUniqueHashes[i].Remove(newIter);
				}
				for (size_t i = 0; i < multiHashIndex; ++i)
					mMultiHashes[i].Remove(newMultiHashIters[i]);
				throw;
			}
			for (size_t i = 0; i < uniqueHashCount; ++i)
			{
				auto oldIter = oldUniqueHashIters[i];
				if (!!oldIter)
					mUniqueHashes[i].Remove(oldIter);
			}
			for (size_t i = 0; i < multiHashCount; ++i)
				mMultiHashes[i].Remove(oldMultiHashIters[i]);
		}

		template<size_t columnCount>
		const UniqueHash* GetHash(const std::array<size_t, columnCount>& sortedOffsets,
			const UniqueHash*) const MOMO_NOEXCEPT
		{
			return pvGetHash(mUniqueHashes, sortedOffsets);
		}

		template<size_t columnCount>
		const MultiHash* GetHash(const std::array<size_t, columnCount>& sortedOffsets,
			const MultiHash*) const MOMO_NOEXCEPT
		{
			return pvGetHash(mMultiHashes, sortedOffsets);
		}

		template<size_t columnCount>
		const UniqueHash* GetFitUniqueHash(const std::array<size_t, columnCount>& sortedOffsets) const MOMO_NOEXCEPT
		{
			for (const UniqueHash& uniqueHash : mUniqueHashes)
			{
				const Offsets& curSortedOffsets = uniqueHash.GetSortedOffsets();
				bool includes = std::includes(sortedOffsets.begin(), sortedOffsets.end(),
					curSortedOffsets.GetBegin(), curSortedOffsets.GetEnd());
				if (includes)
					return &uniqueHash;
			}
			return nullptr;
		}

		template<size_t columnCount>
		const MultiHash* GetFitMultiHash(const std::array<size_t, columnCount>& sortedOffsets) const MOMO_NOEXCEPT
		{
			const MultiHash* resMultiHash = nullptr;
			size_t maxKeyCount = 0;
			for (const MultiHash& multiHash : mMultiHashes)
			{
				const Offsets& curSortedOffsets = multiHash.GetSortedOffsets();
				bool includes = std::includes(sortedOffsets.begin(), sortedOffsets.end(),
					curSortedOffsets.GetBegin(), curSortedOffsets.GetEnd());
				size_t keyCount = multiHash.GetKeyCount();
				if (includes && keyCount > maxKeyCount)
				{
					maxKeyCount = keyCount;
					resMultiHash = &multiHash;
				}
			}
			return resMultiHash;
		}

		template<size_t columnCount>
		static std::array<size_t, columnCount> GetSortedOffsets(
			const std::array<size_t, columnCount>& offsets)
		{
			MOMO_STATIC_ASSERT(columnCount > 0);
			std::array<size_t, columnCount> sortedOffsets = offsets;
			std::sort(sortedOffsets.begin(), sortedOffsets.end());
			MOMO_CHECK(std::unique(sortedOffsets.begin(), sortedOffsets.end()) == sortedOffsets.end());
			return sortedOffsets;
		}

		template<typename Hash>
		static bool HasOffset(const Hash& hash, size_t offset) MOMO_NOEXCEPT
		{
			const Offsets& sortedOffsets = hash.GetSortedOffsets();
			return std::binary_search(sortedOffsets.GetBegin(), sortedOffsets.GetEnd(), offset);
		}

	private:
		MemManagerPtr pvGetMemManagerPtr() const MOMO_NOEXCEPT
		{
			return mOffsetHashCodes.GetMemManager();
		}

		template<typename Hashes, typename... Types>
		const typename Hashes::Item* pvGetHash(const Hashes& hashes,
			const Column<Types>&... columns) const
		{
			static const size_t columnCount = sizeof...(Types);
			std::array<size_t, columnCount> offsets = {{ mColumnList->GetOffset(columns)... }};	// C++11
			std::array<size_t, columnCount> sortedOffsets = GetSortedOffsets(offsets);
			return pvGetHash(hashes, sortedOffsets);
		}

		template<typename Hashes, size_t columnCount>
		const typename Hashes::Item* pvGetHash(const Hashes& hashes,
			const std::array<size_t, columnCount>& sortedOffsets) const
		{
			for (const auto& hash : hashes)
			{
				const Offsets& curSortedOffsets = hash.GetSortedOffsets();
				bool equal = curSortedOffsets.GetCount() == columnCount
					&& std::equal(sortedOffsets.begin(), sortedOffsets.end(), curSortedOffsets.GetBegin());
				if (equal)
					return &hash;
			}
			return nullptr;
		}

		template<typename Hashes, typename Raws, typename... Types>
		const typename Hashes::Item* pvAddHash(Hashes& hashes, const Raws& raws,
			const Column<Types>&... columns)
		{
			const ColumnList* columnList = mColumnList;
			static const size_t columnCount = sizeof...(Types);
			std::array<size_t, columnCount> offsets = {{ columnList->GetOffset(columns)... }};	// C++11
			for (size_t offset : offsets)
			{
				if (columnList->IsMutable(offset))
					throw std::runtime_error("Cannot add index on mutable column");
			}
			std::array<size_t, columnCount> sortedOffsets = GetSortedOffsets(offsets);
			const auto* hash = pvGetHash(hashes, sortedOffsets);
			if (hash != nullptr)
				return hash;
			auto hashFunc = [columnList, offsets] (Raw* raw, size_t* hashCodes)
			{
				if (hashCodes == nullptr)
					return pvGetHashCode<void, Types...>(columnList, raw, offsets.data());
				else
					return pvGetHashCode<void, Types...>(columnList, raw, offsets.data(), hashCodes);
			};
			auto equalFunc = [columnList, offsets] (Raw* raw1, Raw* raw2)
				{ return pvIsEqual<void, Types...>(columnList, raw1, raw2, offsets.data()); };
			typename Hashes::Item newHash(
				Offsets(sortedOffsets.begin(), sortedOffsets.end(), pvGetMemManagerPtr()), hashFunc, equalFunc);
			for (Raw* raw : raws)
				newHash.Add(raw, nullptr);
			hashes.AddBack(std::move(newHash));
			return hashes.GetItems() + hashes.GetCount() - 1;
		}

		template<typename Hashes, typename... Types>
		bool pvRemoveHash(Hashes& hashes, const Column<Types>&... columns)
		{
			const auto* hash = pvGetHash(hashes, columns...);
			if (hash == nullptr)
				return false;
			hashes.Remove(hash - hashes.GetItems(), 1);
			return true;
		}

		template<typename Void, typename Type, typename... Types>
		static size_t pvGetHashCode(const ColumnList* columnList, Raw* raw, const size_t* offsets)
		{
			return pvGetHashCode<Type>(columnList, raw, *offsets)
				+ pvGetHashCode<void, Types...>(columnList, raw, offsets + 1);
		}

		template<typename Void>
		static size_t pvGetHashCode(const ColumnList* /*columnList*/, Raw* /*raw*/,
			const size_t* /*offsets*/) MOMO_NOEXCEPT
		{
			return 0;
		}

		template<typename Void, typename Type, typename... Types>
		static size_t pvGetHashCode(const ColumnList* columnList, Raw* raw,
			const size_t* offsets, size_t* hashCodes)
		{
			size_t& hashCode = hashCodes[*offsets];
			if (hashCode == 0)
				hashCode = pvGetHashCode<Type>(columnList, raw, *offsets);
			return hashCode + pvGetHashCode<void, Types...>(columnList, raw, offsets + 1, hashCodes);	//?
		}

		template<typename Void>
		static size_t pvGetHashCode(const ColumnList* /*columnList*/, Raw* /*raw*/,
			const size_t* /*offsets*/, size_t* /*hashCodes*/) MOMO_NOEXCEPT
		{
			return 0;
		}

		template<size_t number, typename... Types>
		static size_t pvGetHashCode(const OffsetItemTuple<Types...>& tuple,
			typename std::enable_if<(number < sizeof...(Types)), int>::type = 0)
		{
			const auto& pair = std::get<number>(tuple);
			const auto& item = pair.second;
			return pvGetHashCode(item, pair.first) + pvGetHashCode<number + 1>(tuple);	//?
		}

		template<size_t number, typename... Types>
		static size_t pvGetHashCode(const OffsetItemTuple<Types...>& /*tuple*/,
			typename std::enable_if<(number == sizeof...(Types)), int>::type = 0) MOMO_NOEXCEPT
		{
			return 0;
		}

		template<typename Type>
		static size_t pvGetHashCode(const ColumnList* columnList, Raw* raw, size_t offset)
		{
			const Type& item = columnList->template GetByOffset<const Type>(raw, offset);
			return pvGetHashCode(item, offset);
		}

		template<typename Type>
		static size_t pvGetHashCode(const Type& item, size_t /*offset*/)
		{
			return DataTraits::GetHashCode(item);	//?
		}

		template<typename Void, typename Type, typename... Types>
		static bool pvIsEqual(const ColumnList* columnList, Raw* raw1, Raw* raw2,
			const size_t* offsets)
		{
			const Type& item1 = columnList->template GetByOffset<const Type>(raw1, *offsets);
			const Type& item2 = columnList->template GetByOffset<const Type>(raw2, *offsets);
			return DataTraits::IsEqual(item1, item2)
				&& pvIsEqual<void, Types...>(columnList, raw1, raw2, offsets + 1);
		}

		template<typename Void>
		static bool pvIsEqual(const ColumnList* /*columnList*/, Raw* /*raw1*/, Raw* /*raw2*/,
			const size_t* /*offsets*/) MOMO_NOEXCEPT
		{
			return true;
		}

	private:
		const ColumnList* mColumnList;
		UniqueHashes mUniqueHashes;
		MultiHashes mMultiHashes;
		OffsetHashCodes mOffsetHashCodes;
	};
}

} // namespace experimental

} // namespace momo

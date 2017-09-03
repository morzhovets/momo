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

	struct DataHashMultiMapSettings : public momo::HashMultiMapSettings
	{
		static const CheckMode checkMode = CheckMode::assertion;
		static const ExtraCheckMode extraCheckMode = ExtraCheckMode::nothing;
		static const bool checkKeyVersion = false;
		static const bool checkValueVersion = false;
	};

	template<typename TColumnList, typename TKeyIterator>
	class DataMultiHashIterator
	{
	protected:
		typedef TColumnList ColumnList;
		typedef TKeyIterator KeyIterator;
		typedef typename ColumnList::Settings Settings;
		typedef typename ColumnList::Raw Raw;

	public:
		typedef Raw* const& Reference;
		typedef Raw* const* Pointer;

		typedef DataMultiHashIterator ConstIterator;

	public:
		DataMultiHashIterator() MOMO_NOEXCEPT
			: mRawIndex(0)
		{
		}

		DataMultiHashIterator(KeyIterator keyIter, size_t rawIndex) MOMO_NOEXCEPT
			: mKeyIterator(keyIter),
			mRawIndex(rawIndex)
		{
		}

		DataMultiHashIterator& operator+=(ptrdiff_t diff)
		{
			mRawIndex += diff;
			return *this;
		}

		ptrdiff_t operator-(DataMultiHashIterator iter) const
		{
			MOMO_CHECK(mKeyIterator == iter.mKeyIterator);
			return mRawIndex - iter.mRawIndex;
		}

		Pointer operator->() const
		{
			MOMO_CHECK(!!mKeyIterator);
			MOMO_CHECK(mRawIndex <= mKeyIterator->values.GetCount());
			if (mRawIndex > 0)
				return &mKeyIterator->values[mRawIndex - 1];
			else
				return &mKeyIterator->key.raw;
		}

		bool operator==(DataMultiHashIterator iter) const MOMO_NOEXCEPT
		{
			return mKeyIterator == iter.mKeyIterator
				&& mRawIndex == iter.mRawIndex;
		}

		bool operator<(DataMultiHashIterator iter) const
		{
			MOMO_CHECK(mKeyIterator == iter.mKeyIterator);
			return mRawIndex < iter.mRawIndex;
		}

		MOMO_MORE_ARRAY_ITERATOR_OPERATORS(DataMultiHashIterator)

	private:
		KeyIterator mKeyIterator;
		size_t mRawIndex;
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
			momo::internal::NestedArraySettings<ArraySettings<internalCapacity, false>>>;	//?

		typedef Array<size_t> Offsets;

		typedef std::function<size_t(Raw*, size_t*)> HashFunc;
		typedef std::function<bool(Raw*, Raw*)> EqualFunc;

		struct HashRawKey
		{
			Raw* raw;
		};

		template<typename... Types>
		struct HashTupleKey
		{
			OffsetItemTuple<Types...> tuple;
			size_t hashCode;
			const ColumnList* columnList;
		};

		class HashTraits : public momo::HashTraitsOpen<HashRawKey>
		{
		public:
			template<typename KeyArg>
			struct IsValidKeyArg : public std::false_type
			{
			};

			template<typename... Types>
			struct IsValidKeyArg<HashTupleKey<Types...>> : public std::true_type
			{
			};

		public:
			HashTraits(HashFunc&& hashFunc, EqualFunc&& equalFunc)
				: mHashFunc(std::move(hashFunc)),
				mEqualFunc(std::move(equalFunc))
			{
			}

			size_t GetHashCode(const HashRawKey& key) const
			{
				return mHashFunc(key.raw, nullptr);
			}

			template<typename... Types>
			size_t GetHashCode(const HashTupleKey<Types...>& key) const MOMO_NOEXCEPT
			{
				return key.hashCode;
			}

			bool IsEqual(const HashRawKey& key1, const HashRawKey& key2) const
			{
				return mEqualFunc(key1.raw, key2.raw);
			}

			template<typename... Types>
			bool IsEqual(const HashTupleKey<Types...>& key1, const HashRawKey& key2) const
			{
				return pvIsEqual<0>(key1, key2);
			}

		private:
			template<size_t index, typename... Types>
			bool pvIsEqual(const HashTupleKey<Types...>& key1, const HashRawKey& key2,
				typename std::enable_if<(index < sizeof...(Types)), int>::type = 0) const
			{
				const auto& pair = std::get<index>(key1.tuple);
				const auto& item1 = pair.second;
				typedef typename std::decay<decltype(item1)>::type Type;
				const Type& item2 = key1.columnList->template GetByOffset<const Type>(key2.raw, pair.first);
				return DataTraits::IsEqual(item1, item2) && pvIsEqual<index + 1>(key1, key2);
			}

			template<size_t index, typename... Types>
			bool pvIsEqual(const HashTupleKey<Types...>& /*key1*/, const HashRawKey& /*key2*/,
				typename std::enable_if<(index == sizeof...(Types)), int>::type = 0) const MOMO_NOEXCEPT
			{
				return true;
			}

		private:
			HashFunc mHashFunc;
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
			Raw* raw;	//?
			const UniqueHash& uniqueHash;
		};

		class UniqueHash
		{
		private:
			typedef momo::HashSet<HashRawKey, HashTraits, MemManagerPtr,
				HashSetItemTraits<HashRawKey, HashRawKey, MemManagerPtr>, DataHashSetSettings> HashSet;

			typedef typename HashSet::ConstIterator Iterator;

		public:
			class RawBounds
			{
			public:
				typedef Raw* const* Iterator;

				typedef RawBounds ConstBounds;

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
				mHashSet(typename HashSet::HashTraits(std::move(hashFunc), std::move(equalFunc)),
					MemManagerPtr(mSortedOffsets.GetMemManager()))
			{
			}

			UniqueHash(UniqueHash&& uniqueHash) MOMO_NOEXCEPT
				: mSortedOffsets(std::move(uniqueHash.mSortedOffsets)),
				mHashSet(std::move(uniqueHash.mHashSet)),
				mIterator(uniqueHash.mIterator)
			{
			}

			UniqueHash(const UniqueHash&) = delete;

			~UniqueHash() MOMO_NOEXCEPT
			{
			}

			UniqueHash& operator=(UniqueHash&& uniqueHash) MOMO_NOEXCEPT
			{
				mSortedOffsets = std::move(uniqueHash.mSortedOffsets);
				mHashSet = std::move(uniqueHash.mHashSet);
				mIterator = uniqueHash.mIterator;
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

			void Clear() MOMO_NOEXCEPT
			{
				mHashSet.Clear();
			}

			RawBounds Find(Raw* raw) const
			{
				Iterator iter = mHashSet.Find({ raw });
				return RawBounds(!!iter ? iter->raw : nullptr);
			}

			template<typename... Types>
			RawBounds Find(const HashTupleKey<Types...>& hashTupleKey) const
			{
				Iterator iter = mHashSet.Find(hashTupleKey);
				return RawBounds(!!iter ? iter->raw : nullptr);
			}

			void FindExisting(Raw* raw) MOMO_NOEXCEPT
			{
				MOMO_ASSERT(!mIterator);
				try
				{
					mIterator = mHashSet.Find({ raw });
					MOMO_ASSERT(!!mIterator);
				}
				catch (...)
				{
					mIterator = mHashSet.GetBegin();
					while (mIterator->raw != raw)
						++mIterator;
				}
			}

			void Add(Raw* raw, size_t* /*hashCodes*/)
			{
				MOMO_ASSERT(!mIterator);
				auto insRes = mHashSet.Insert({ raw });
				if (!insRes.inserted)
					throw UniqueIndexViolation(insRes.iterator->raw, *this);
				mIterator = insRes.iterator;
			}

			void Insert(Raw* raw)
			{
				MOMO_ASSERT(!mIterator);
				mIterator = mHashSet.Insert({ raw }).iterator;
			}

			void Remove() MOMO_NOEXCEPT
			{
				MOMO_ASSERT(!!mIterator);
				mHashSet.Remove(mIterator);
				mIterator = Iterator();
			}

			void ResetRaw(Raw* raw) MOMO_NOEXCEPT
			{
				MOMO_ASSERT(!!mIterator);
				mHashSet.ResetKey(mIterator, { raw });
				mIterator = Iterator();
			}

			void Accept() MOMO_NOEXCEPT
			{
				MOMO_ASSERT(!!mIterator);
				mIterator = Iterator();
			}

			Raw* GetCurrentRaw() const MOMO_NOEXCEPT
			{
				MOMO_ASSERT(!!mIterator);
				return mIterator->raw;
			}

			template<typename RawFilter>
			void FilterRaws(RawFilter rawFilter) MOMO_NOEXCEPT
			{
				Iterator iter = mHashSet.GetBegin();
				while (!!iter)
				{
					if (rawFilter(iter->raw))
						++iter;
					else
						iter = mHashSet.Remove(iter);
				}
			}

		private:
			Offsets mSortedOffsets;
			HashSet mHashSet;
			Iterator mIterator;
		};

		class MultiHash
		{
		private:
			typedef momo::HashMultiMap<HashRawKey, Raw*, HashTraits, MemManagerPtr,
				HashMultiMapKeyValueTraits<HashRawKey, Raw*, MemManagerPtr>,
				DataHashMultiMapSettings> HashMultiMap;

			typedef typename HashMultiMap::ConstKeyIterator KeyIterator;

		public:
			class RawBounds
			{
			public:
				typedef DataMultiHashIterator<ColumnList, KeyIterator> Iterator;

				typedef RawBounds ConstBounds;

			public:
				RawBounds(KeyIterator keyIter = KeyIterator()) MOMO_NOEXCEPT
					: mKeyIterator(keyIter)
				{
				}

				Iterator GetBegin() const MOMO_NOEXCEPT
				{
					return Iterator(mKeyIterator, 0);
				}

				Iterator GetEnd() const MOMO_NOEXCEPT
				{
					return GetBegin() + GetCount();
				}

				MOMO_FRIENDS_BEGIN_END(const RawBounds&, Iterator)

				size_t GetCount() const MOMO_NOEXCEPT
				{
					return !!mKeyIterator ? mKeyIterator->values.GetCount() + 1 : 0;
				}

			private:
				KeyIterator mKeyIterator;
			};

		public:
			MultiHash(Offsets&& sortedOffsets, HashFunc&& hashFunc, EqualFunc&& equalFunc)
				: mSortedOffsets(std::move(sortedOffsets)),
				mHashMultiMap(typename HashMultiMap::HashTraits(std::move(hashFunc), std::move(equalFunc)),
					MemManagerPtr(mSortedOffsets.GetMemManager())),
				mRawIndex(0)
			{
			}

			MultiHash(MultiHash&& multiHash) MOMO_NOEXCEPT
				: mSortedOffsets(std::move(multiHash.mSortedOffsets)),
				mHashMultiMap(std::move(multiHash.mHashMultiMap)),
				mKeyIterator(multiHash.mKeyIterator),
				mRawIndex(multiHash.mRawIndex)
			{
			}

			MultiHash(const MultiHash&) = delete;

			~MultiHash() MOMO_NOEXCEPT
			{
			}

			MultiHash& operator=(MultiHash&& multiHash) MOMO_NOEXCEPT
			{
				mSortedOffsets = std::move(multiHash.mSortedOffsets);
				mHashMultiMap = std::move(multiHash.mHashMultiMap);
				mKeyIterator = multiHash.mKeyIterator;
				mRawIndex = multiHash.mRawIndex;
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

			void Clear() MOMO_NOEXCEPT
			{
				mHashMultiMap.Clear();
			}

			template<typename... Types>
			RawBounds Find(const HashTupleKey<Types...>& hashTupleKey) const
			{
				return RawBounds(mHashMultiMap.Find(hashTupleKey));
			}

			void FindExisting(Raw* raw) MOMO_NOEXCEPT
			{
				MOMO_ASSERT(!mKeyIterator);
				try
				{
					mKeyIterator = mHashMultiMap.Find({ raw });
					MOMO_ASSERT(!!mKeyIterator);
					pvFindExisting(raw);
				}
				catch (...)
				{
					mKeyIterator = mHashMultiMap.GetKeyBounds().GetBegin();
					while (!pvFindExisting(raw))
						++mKeyIterator;
				}
			}

			void Add(Raw* raw, size_t* /*hashCodes*/)
			{
				MOMO_ASSERT(!mKeyIterator);
				mKeyIterator = mHashMultiMap.InsertKey({ raw });
				if (mKeyIterator->key.raw != raw)
					mHashMultiMap.Add(mKeyIterator, raw);
				mRawIndex = mKeyIterator->values.GetCount();
			}

			void Remove() MOMO_NOEXCEPT
			{
				MOMO_ASSERT(!!mKeyIterator);
				if (mRawIndex > 0)
				{
					mHashMultiMap.Remove(mKeyIterator, mRawIndex - 1);
				}
				else if (mKeyIterator->values.GetCount() > 0)
				{
					size_t valueCount = mKeyIterator->values.GetCount();
					mHashMultiMap.ResetKey(mKeyIterator, { mKeyIterator->values[valueCount - 1] });
					mHashMultiMap.Remove(mKeyIterator, valueCount - 1);
				}
				else
				{
					mHashMultiMap.RemoveKey(mKeyIterator);
				}
				mKeyIterator = KeyIterator();
			}

			void Accept() MOMO_NOEXCEPT
			{
				MOMO_ASSERT(!!mKeyIterator);
				mKeyIterator = KeyIterator();
			}

			template<typename RawFilter>
			void FilterRaws(RawFilter rawFilter) MOMO_NOEXCEPT
			{
				KeyIterator keyIter = mHashMultiMap.GetKeyBounds().GetBegin();
				while (!!keyIter)
				{
					size_t valueIndex = 0;
					while (valueIndex < keyIter->values.GetCount())
					{
						if (rawFilter(keyIter->values[valueIndex]))
							++valueIndex;
						else
							mHashMultiMap.Remove(keyIter, valueIndex);
					}
					if (rawFilter(keyIter->key.raw))
					{
						++keyIter;
						continue;
					}
					size_t valueCount = keyIter->values.GetCount();
					if (valueCount == 0)
					{
						keyIter = mHashMultiMap.RemoveKey(keyIter).GetKeyIterator();
						continue;
					}
					mHashMultiMap.ResetKey(keyIter, { keyIter->values[valueCount - 1] });
					mHashMultiMap.Remove(keyIter, valueCount - 1);
					++keyIter;
				}
			}

		private:
			bool pvFindExisting(Raw* raw) MOMO_NOEXCEPT
			{
				if (mKeyIterator->key.raw == raw)
				{
					mRawIndex = 0;
					return true;
				}
				auto raws = mKeyIterator->values;
				Raw* const* rawPtr = std::find(raws.GetBegin(), raws.GetEnd(), raw);
				if (rawPtr == raws.GetEnd())
					return false;
				mRawIndex = rawPtr - raws.GetBegin() + 1;
				return true;
			}

		private:
			Offsets mSortedOffsets;
			HashMultiMap mHashMultiMap;
			KeyIterator mKeyIterator;
			size_t mRawIndex;
		};

	private:
		typedef Array<UniqueHash> UniqueHashes;

		typedef Array<MultiHash> MultiHashes;

		typedef Array<size_t> OffsetHashCodes;

	public:
		DataIndexes(const ColumnList* columnList, MemManager& memManager)
			: mColumnList(columnList),
			mUniqueHashes(MemManagerPtr(memManager)),
			mMultiHashes(MemManagerPtr(memManager))
		{
		}

		DataIndexes(DataIndexes&& indexes) MOMO_NOEXCEPT
			: mColumnList(indexes.mColumnList),
			mUniqueHashes(std::move(indexes.mUniqueHashes)),
			mMultiHashes(std::move(indexes.mMultiHashes))
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
			size_t uniqueHashIndex = 0;
			size_t multiHashCount = mMultiHashes.GetCount();
			size_t multiHashIndex = 0;
			size_t* hashCodes = nullptr;	//?
			try
			{
				for (; uniqueHashIndex < uniqueHashCount; ++uniqueHashIndex)
					mUniqueHashes[uniqueHashIndex].Add(raw, hashCodes);
				for (; multiHashIndex < multiHashCount; ++multiHashIndex)
					mMultiHashes[multiHashIndex].Add(raw, hashCodes);
			}
			catch (...)
			{
				for (size_t i = 0; i < uniqueHashIndex; ++i)
					mUniqueHashes[i].Remove();
				for (size_t i = 0; i < multiHashIndex; ++i)
					mMultiHashes[i].Remove();
				throw;
			}
			for (UniqueHash& uniqueHash : mUniqueHashes)
				uniqueHash.Accept();
			for (MultiHash& multiHash : mMultiHashes)
				multiHash.Accept();
		}

		void RemoveRaw(Raw* raw)
		{
			for (UniqueHash& uniqueHash : mUniqueHashes)
			{
				uniqueHash.FindExisting(raw);
				uniqueHash.Remove();
			}
			for (MultiHash& multiHash : mMultiHashes)
			{
				multiHash.FindExisting(raw);
				multiHash.Remove();
			}
		}

		void UpdateRaw(Raw* oldRaw, Raw* newRaw)
		{
			size_t uniqueHashCount = mUniqueHashes.GetCount();
			size_t uniqueHashIndex = 0;
			size_t multiHashCount = mMultiHashes.GetCount();
			size_t multiHashIndex = 0;
			try
			{
				for (; uniqueHashIndex < uniqueHashCount; ++uniqueHashIndex)
				{
					mUniqueHashes[uniqueHashIndex].Insert(newRaw);
					Raw* raw = mUniqueHashes[uniqueHashIndex].GetCurrentRaw();
					if (raw != oldRaw && raw != newRaw)
						throw UniqueIndexViolation(raw, mUniqueHashes[uniqueHashIndex]);
				}
				for (; multiHashIndex < multiHashCount; ++multiHashIndex)
					mMultiHashes[multiHashIndex].Add(newRaw, nullptr);
			}
			catch (...)
			{
				for (size_t i = 0; i < uniqueHashIndex; ++i)
				{
					if (mUniqueHashes[i].GetCurrentRaw() == newRaw)
						mUniqueHashes[i].Remove();
					else
						mUniqueHashes[i].Accept();
				}
				for (size_t i = 0; i < multiHashIndex; ++i)
					mMultiHashes[i].Remove();
				throw;
			}
			for (UniqueHash& uniqueHash : mUniqueHashes)
			{
				if (uniqueHash.GetCurrentRaw() == oldRaw)
				{
					uniqueHash.ResetRaw(newRaw);
				}
				else
				{
					uniqueHash.Accept();
					uniqueHash.FindExisting(oldRaw);
					uniqueHash.Remove();
				}
			}
			for (MultiHash& multiHash : mMultiHashes)
			{
				multiHash.Accept();
				multiHash.FindExisting(oldRaw);
				multiHash.Remove();
			}
		}

		template<typename RawFilter>
		void FilterRaws(RawFilter rawFilter) MOMO_NOEXCEPT
		{
			for (UniqueHash& uniqueHash : mUniqueHashes)
				uniqueHash.FilterRaws(rawFilter);
			for (MultiHash& multiHash : mMultiHashes)
				multiHash.FilterRaws(rawFilter);
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
			return mUniqueHashes.GetMemManager();
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
			Offsets newHashOffsets(sortedOffsets.begin(), sortedOffsets.end(), pvGetMemManagerPtr());
			typename Hashes::Item newHash(std::move(newHashOffsets), hashFunc, equalFunc);
			for (Raw* raw : raws)
			{
				newHash.Add(raw, nullptr);
				newHash.Accept();
			}
			hashes.Reserve(hashes.GetCount() + 1);
			hashes.AddBackNogrow(std::move(newHash));
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

		template<size_t index, typename... Types>
		static size_t pvGetHashCode(const OffsetItemTuple<Types...>& tuple,
			typename std::enable_if<(index < sizeof...(Types)), int>::type = 0)
		{
			const auto& pair = std::get<index>(tuple);
			const auto& item = pair.second;
			return pvGetHashCode(item, pair.first) + pvGetHashCode<index + 1>(tuple);	//?
		}

		template<size_t index, typename... Types>
		static size_t pvGetHashCode(const OffsetItemTuple<Types...>& /*tuple*/,
			typename std::enable_if<(index == sizeof...(Types)), int>::type = 0) MOMO_NOEXCEPT
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
	};
}

} // namespace experimental

} // namespace momo

namespace std
{
	template<typename CL, typename KI>
	struct iterator_traits<momo::experimental::internal::DataMultiHashIterator<CL, KI>>
		: public iterator_traits<typename CL::Raw* const*>
	{
	};
} // namespace std

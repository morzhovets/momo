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
	template<typename TColumnList, typename TDataTraits, typename TMemManager>
	class DataIndexes
	{
	public:
		typedef TColumnList ColumnList;
		typedef TDataTraits DataTraits;
		typedef TMemManager MemManager;
		typedef typename ColumnList::Raw Raw;

		template<typename Type>
		using Column = typename ColumnList::template Column<Type>;

		template<typename... Types>
		using OffsetItemTuple = std::tuple<std::pair<size_t, const Types&>...>;

	private:
		typedef momo::internal::MemManagerPtr<MemManager> MemManagerPtr;

		template<typename Item, size_t internalCapacity = 0>
		using Array = momo::Array<Item, MemManagerPtr, ArrayItemTraits<Item>,
			momo::internal::NestedArraySettings<ArraySettings<internalCapacity>>>;

		typedef Array<size_t> Offsets;

		typedef std::function<size_t(const Raw*, size_t*)> HashFunc;
		typedef std::function<bool(const Raw*, const Raw*)> EqualFunc;

		template<typename... Types>
		struct HashTupleKey
		{
			OffsetItemTuple<Types...> tuple;
			size_t hashCode;
			const ColumnList* columnList;
		};

		template<typename HashRawKey>
		struct HashBucketStater
		{
			static unsigned char GetState(const HashRawKey* key) noexcept
			{
				return (key->raw != nullptr) ? (unsigned char)1 : (unsigned char)key->hashCode;
			}

			template<typename Item>
			static unsigned char GetState(const Item* item) noexcept
			{
				return GetState(item->GetKeyPtr());
			}

			static void SetState(HashRawKey* key, unsigned char state) noexcept
			{
				MOMO_ASSERT(state != (unsigned char)1 || key->raw != nullptr);
				if (state != (unsigned char)1)
				{
					key->raw = nullptr;
					key->hashCode = (size_t)state;
				}
			}

			template<typename Item>
			static void SetState(Item* item, unsigned char state) noexcept
			{
				return SetState(item->GetKeyPtr(), state);
			}
		};

		template<typename HashRawKey>
		class HashTraits : public momo::HashTraits<HashRawKey, HashBucketOneI<HashBucketStater<HashRawKey>>>
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
			explicit HashTraits(EqualFunc&& equalFunc) noexcept
				: mEqualFunc(std::move(equalFunc))
			{
			}

			size_t GetHashCode(const HashRawKey& key) const noexcept
			{
				return key.hashCode;
			}

			template<typename... Types>
			size_t GetHashCode(const HashTupleKey<Types...>& key) const noexcept
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
				return key1.hashCode == key2.hashCode && _IsEqual<0>(key1, key2);
			}

		private:
			template<size_t index, typename... Types>
			bool _IsEqual(const HashTupleKey<Types...>& key1, const HashRawKey& key2,
				typename std::enable_if<(index < sizeof...(Types)), int>::type = 0) const
			{
				const auto& pair = std::get<index>(key1.tuple);
				const auto& item1 = pair.second;
				typedef typename std::decay<decltype(item1)>::type Type;
				const Type& item2 = key1.columnList->template GetByOffset<Type>(key2.raw, pair.first);
				return DataTraits::IsEqual(item1, item2) && _IsEqual<index + 1>(key1, key2);
			}

			template<size_t index, typename... Types>
			bool _IsEqual(const HashTupleKey<Types...>& /*key1*/, const HashRawKey& /*key2*/,
				typename std::enable_if<(index == sizeof...(Types)), int>::type = 0) const noexcept
			{
				return true;
			}

		private:
			EqualFunc mEqualFunc;
		};

		class UniqueHash
		{
		private:
			struct HashRawKey
			{
				Raw* raw;
				size_t hashCode;
			};

			//? HashSetSettings
			typedef momo::HashSet<HashRawKey, HashTraits<HashRawKey>, MemManagerPtr> HashSet;

		public:
			typedef typename HashSet::ConstIterator Iterator;

			class RawBounds
			{
			public:
				typedef Raw* const* Iterator;

			public:
				explicit RawBounds(Raw* raw) noexcept
				{
					mRaws[0] = raw;
				}

				Iterator GetBegin() const noexcept
				{
					return mRaws;
				}

				Iterator GetEnd() const noexcept
				{
					return mRaws + ((mRaws[0] != nullptr) ? 1 : 0);
				}

				MOMO_FRIENDS_BEGIN_END(const RawBounds&, Iterator)

			private:
				Raw* mRaws[1];	//?
			};

		public:
			UniqueHash(Offsets&& sortedOffsets, HashFunc&& hashFunc, EqualFunc&& equalFunc) noexcept
				: mSortedOffsets(std::move(sortedOffsets)),
				mHashFunc(std::move(hashFunc)),
				mHashSet(typename HashSet::HashTraits(std::move(equalFunc)),
					MemManagerPtr(mSortedOffsets.GetMemManager()))
			{
			}

			UniqueHash(UniqueHash&& uniqueHash) noexcept
				: mSortedOffsets(std::move(uniqueHash.mSortedOffsets)),
				mHashFunc(std::move(uniqueHash.mHashFunc)),
				mHashSet(std::move(uniqueHash.mHashSet))
			{
			}

			~UniqueHash() noexcept
			{
			}

			UniqueHash& operator=(UniqueHash&& uniqueHash) noexcept
			{
				mSortedOffsets = std::move(uniqueHash.mSortedOffsets);
				mHashFunc = std::move(uniqueHash.mHashFunc);
				mHashSet = std::move(uniqueHash.mHashSet);
				return *this;
			}

			const Offsets& GetSortedOffsets() const noexcept
			{
				return mSortedOffsets;
			}

			Iterator Find(Raw* raw, size_t* hashCodes)
			{
				size_t hashCode = mHashFunc(raw, hashCodes);
				Iterator iter = mHashSet.Find({ raw, hashCode });
				MOMO_ASSERT(!!iter);
				return iter;
			}

			template<typename... Types>
			RawBounds Find(const HashTupleKey<Types...>& hashTupleKey) const
			{
				Iterator iter = mHashSet.Find(hashTupleKey);
				return RawBounds(!!iter ? iter->raw : nullptr);
			}

			void Clear() noexcept
			{
				mHashSet.Clear();
			}

			Iterator Add(Raw* raw, size_t* hashCodes)
			{
				size_t hashCode = mHashFunc(raw, hashCodes);
				auto insRes = mHashSet.Insert({ raw, hashCode });
				if (!insRes.inserted)
					throw std::runtime_error("Unique index violation");
				return insRes.iterator;
			}

			void Remove(Iterator iter) noexcept
			{
				mHashSet.Remove(iter);
			}

		private:
			Offsets mSortedOffsets;
			HashFunc mHashFunc;
			HashSet mHashSet;
		};

		typedef Array<UniqueHash> UniqueHashes;
		typedef Array<typename UniqueHash::Iterator, 8> UniqueHashIterators;

		class MultiHash
		{
		private:
			struct HashRawKey
			{
				Raw* raw;
				size_t hashCode;
			};

			//? HashMultiMapSettings, HashMapSettings
			typedef momo::HashMultiMap<HashRawKey, Raw*, HashTraits<HashRawKey>, MemManagerPtr> HashMultiMap;
			typedef momo::HashMap<Raw*, size_t, momo::HashTraits<Raw*>, MemManagerPtr> HashMap;	//?

			static const size_t rawFastCount = 8;

		public:
			typedef typename HashMultiMap::Iterator Iterator;

			typedef typename HashMultiMap::ConstValueBounds RawBounds;

		public:
			MultiHash(Offsets&& sortedOffsets, HashFunc&& hashFunc, EqualFunc&& equalFunc) noexcept
				: mSortedOffsets(std::move(sortedOffsets)),
				mHashFunc(std::move(hashFunc)),
				mHashMultiMap(typename HashMultiMap::HashTraits(std::move(equalFunc)),
					MemManagerPtr(mSortedOffsets.GetMemManager())),
				mHashMap(typename HashMap::HashTraits(), MemManagerPtr(mSortedOffsets.GetMemManager()))
			{
			}

			MultiHash(MultiHash&& multiHash) noexcept
				: mSortedOffsets(std::move(multiHash.mSortedOffsets)),
				mHashFunc(std::move(multiHash.mHashFunc)),
				mHashMultiMap(std::move(multiHash.mHashMultiMap)),
				mHashMap(std::move(multiHash.mHashMap))
			{
			}

			~MultiHash() noexcept
			{
			}

			MultiHash& operator=(MultiHash&& multiHash) noexcept
			{
				mSortedOffsets = std::move(multiHash.mSortedOffsets);
				mHashFunc = std::move(multiHash.mHashFunc);
				mHashMultiMap = std::move(multiHash.mHashMultiMap);
				mHashMap = std::move(multiHash.mHashMap);
				return *this;
			}

			const Offsets& GetSortedOffsets() const noexcept
			{
				return mSortedOffsets;
			}

			size_t GetKeyCount() const noexcept
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
				size_t rawNum = std::find(raws.GetBegin(), raws.GetEnd(), raw) - raws.GetBegin();
				return mHashMultiMap.MakeIterator(keyIter, rawNum);
			}

			template<typename... Types>
			RawBounds Find(const HashTupleKey<Types...>& hashTupleKey) const
			{
				auto keyIter = mHashMultiMap.Find(hashTupleKey);
				return !!keyIter ? keyIter->values : RawBounds();
			}

			void Clear() noexcept
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

			void Remove(Iterator iter) noexcept
			{
				Raw* raw = iter->value;
				mHashMap.Remove(raw);
				auto keyIter = iter.GetKeyIterator();
				mHashMultiMap.Remove(iter);
				if (keyIter->key.raw == raw)
				{
					auto raws = keyIter->values;
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

		typedef Array<MultiHash> MultiHashes;
		typedef Array<typename MultiHash::Iterator, 8> MultiHashIterators;

		typedef Array<size_t> OffsetHashCodes;

	public:
		DataIndexes(const ColumnList* columnList, MemManager& memManager)
			: mColumnList(columnList),
			mUniqueHashes(MemManagerPtr(memManager)),
			mMultiHashes(MemManagerPtr(memManager)),
			mOffsetHashCodes(columnList->GetTotalSize(), (size_t)0, MemManagerPtr(memManager))
		{
		}

		DataIndexes(DataIndexes&& indexes) noexcept
			: mColumnList(indexes.mColumnList),
			mUniqueHashes(std::move(indexes.mUniqueHashes)),
			mMultiHashes(std::move(indexes.mMultiHashes)),
			mOffsetHashCodes(std::move(indexes.mOffsetHashCodes))
		{
		}

		DataIndexes(const DataIndexes&) = delete;

		~DataIndexes() noexcept
		{
		}

		//DataIndexes& operator=(DataIndexes&& indexes) noexcept
		//{
		//	DataIndexes(std::move(indexes)).Swap(*this);
		//	return *this;
		//}

		DataIndexes& operator=(const DataIndexes&) = delete;

		void Swap(DataIndexes& indexes) noexcept
		{
			std::swap(mColumnList, indexes.mColumnList);
			mUniqueHashes.Swap(indexes.mUniqueHashes);
			mMultiHashes.Swap(indexes.mMultiHashes);
			mOffsetHashCodes.Swap(indexes.mOffsetHashCodes);
		}

		template<typename... Types>
		bool HasUniqueHash(const Column<Types>&... columns) const
		{
			return _FindHash(mUniqueHashes, columns...) != nullptr;
		}

		template<typename... Types>
		bool HasMultiHash(const Column<Types>&... columns) const
		{
			return _FindHash(mMultiHashes, columns...) != nullptr;
		}

		template<typename Raws, typename... Types>
		bool AddUniqueHash(const Raws& raws, const Column<Types>&... columns)
		{
			return _AddHash(mUniqueHashes, raws, columns...);
		}

		template<typename Raws, typename... Types>
		bool AddMultiHash(const Raws& raws, const Column<Types>&... columns)
		{
			return _AddHash(mMultiHashes, raws, columns...);
		}

		template<typename... Types>
		bool RemoveUniqueHash(const Column<Types>&... columns)
		{
			return _RemoveHash(mUniqueHashes, columns...);
		}

		template<typename... Types>
		bool RemoveMultiHash(const Column<Types>&... columns)
		{
			return _RemoveHash(mMultiHashes, columns...);
		}

		template<typename Hash, typename... Types>
		typename Hash::RawBounds FindRaws(const Hash& hash, const OffsetItemTuple<Types...>& tuple) const
		{
			HashTupleKey<Types...> hashTupleKey{ tuple, _GetHashCode<0>(tuple), mColumnList };
			return hash.Find(hashTupleKey);
		}

		void Clear() noexcept
		{
			for (UniqueHash& uniqueHash : mUniqueHashes)
				uniqueHash.Clear();
			for (MultiHash& multiHash : mMultiHashes)
				multiHash.Clear();
		}

		void AddRaw(Raw* raw)
		{
			size_t uniqueHashCount = mUniqueHashes.GetCount();
			UniqueHashIterators uniqueHashIters(uniqueHashCount, _GetMemManagerPtr());
			size_t uniqueHashIndex = 0;
			size_t multiHashCount = mMultiHashes.GetCount();
			MultiHashIterators multiHashIters(multiHashCount, _GetMemManagerPtr());
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
			UniqueHashIterators uniqueHashIters(uniqueHashCount, _GetMemManagerPtr());
			size_t multiHashCount = mMultiHashes.GetCount();
			MultiHashIterators multiHashIters(multiHashCount, _GetMemManagerPtr());
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

		template<size_t columnCount>
		const UniqueHash* FindFitUniqueHash(const std::array<size_t, columnCount>& sortedOffsets) const noexcept
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
		const MultiHash* FindFitMultiHash(const std::array<size_t, columnCount>& sortedOffsets) const noexcept
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
			MOMO_ASSERT(std::unique(sortedOffsets.begin(), sortedOffsets.end()) == sortedOffsets.end());
			return sortedOffsets;
		}

		template<typename Hash>
		static bool HasOffset(const Hash& hash, size_t offset) noexcept
		{
			const Offsets& sortedOffsets = hash.GetSortedOffsets();
			return std::binary_search(sortedOffsets.GetBegin(), sortedOffsets.GetEnd(), offset);
		}

	private:
		MemManagerPtr _GetMemManagerPtr() const noexcept
		{
			return mOffsetHashCodes.GetMemManager();
		}

		template<typename Hashes, typename... Types>
		const typename Hashes::Item* _FindHash(const Hashes& hashes, const Column<Types>&... columns) const
		{
			static const size_t columnCount = sizeof...(Types);
			std::array<size_t, columnCount> offsets = {{ mColumnList->GetOffset(columns)... }};	// C++11
			std::array<size_t, columnCount> sortedOffsets = GetSortedOffsets(offsets);
			return _FindHash(hashes, sortedOffsets);
		}

		template<typename Hashes, size_t columnCount>
		const typename Hashes::Item* _FindHash(const Hashes& hashes,
			const std::array<size_t, columnCount>& sortedOffsets) const
		{
			for (const auto& hash : hashes)
			{
				const Offsets& curSortedOffsets = hash.GetSortedOffsets();
				bool equal = std::equal(curSortedOffsets.GetBegin(), curSortedOffsets.GetEnd(),
					sortedOffsets.begin(), sortedOffsets.end());
				if (equal)
					return &hash;
			}
			return nullptr;
		}

		template<typename Hashes, typename Raws, typename... Types>
		bool _AddHash(Hashes& hashes, const Raws& raws, const Column<Types>&... columns)
		{
			static const size_t columnCount = sizeof...(Types);
			std::array<size_t, columnCount> offsets = {{ mColumnList->GetOffset(columns)... }};	// C++11
			for (size_t offset : offsets)
			{
				if (mColumnList->IsMutable(offset))
					throw std::runtime_error("Cannot add index on mutable column");
			}
			std::array<size_t, columnCount> sortedOffsets = GetSortedOffsets(offsets);
			if (_FindHash(hashes, sortedOffsets) != nullptr)
				return false;
			auto hashFunc = [this, offsets] (const Raw* raw, size_t* hashCodes)
			{
				if (hashCodes == nullptr)
					return _GetHashCode<void, Types...>(raw, offsets.data());
				else
					return _GetHashCode<void, Types...>(raw, offsets.data(), hashCodes);
			};
			auto equalFunc = [this, offsets] (const Raw* raw1, const Raw* raw2)
				{ return _IsEqual<void, Types...>(raw1, raw2, offsets.data()); };
			typename Hashes::Item hash(Offsets(sortedOffsets.begin(), sortedOffsets.end(), _GetMemManagerPtr()),
				hashFunc, equalFunc);
			for (Raw* raw : raws)
				hash.Add(raw, nullptr);
			hashes.AddBack(std::move(hash));
			return true;
		}

		template<typename Hashes, typename... Types>
		bool _RemoveHash(Hashes& hashes, const Column<Types>&... columns)
		{
			const auto* hash = _FindHash(hashes, columns...);
			if (hash == nullptr)
				return false;
			hashes.Remove(hash - hashes.GetItems(), 1);
			return true;
		}

		template<typename Void, typename Type, typename... Types>
		size_t _GetHashCode(const Raw* raw, const size_t* offsets) const
		{
			return _GetHashCode<Type>(raw, *offsets)
				+ _GetHashCode<void, Types...>(raw, offsets + 1);
		}

		template<typename Void>
		size_t _GetHashCode(const Raw* /*raw*/, const size_t* /*offsets*/) const noexcept
		{
			return 0;
		}

		template<typename Void, typename Type, typename... Types>
		size_t _GetHashCode(const Raw* raw, const size_t* offsets, size_t* hashCodes) const
		{
			size_t& hashCode = hashCodes[*offsets];
			if (hashCode == 0)
				hashCode = _GetHashCode<Type>(raw, *offsets);
			return hashCode + _GetHashCode<void, Types...>(raw, offsets + 1, hashCodes);	//?
		}

		template<typename Void>
		size_t _GetHashCode(const Raw* /*raw*/, const size_t* /*offsets*/,
			size_t* /*hashCodes*/) const noexcept
		{
			return 0;
		}

		template<size_t index, typename... Types>
		static size_t _GetHashCode(const OffsetItemTuple<Types...>& tuple,
			typename std::enable_if<(index < sizeof...(Types)), int>::type = 0)
		{
			const auto& pair = std::get<index>(tuple);
			const auto& item = pair.second;
			return _GetHashCode(item, pair.first) + _GetHashCode<index + 1>(tuple);	//?
		}

		template<size_t index, typename... Types>
		static size_t _GetHashCode(const OffsetItemTuple<Types...>& /*tuple*/,
			typename std::enable_if<(index == sizeof...(Types)), int>::type = 0) noexcept
		{
			return 0;
		}

		template<typename Type>
		size_t _GetHashCode(const Raw* raw, size_t offset) const
		{
			const Type& item = mColumnList->template GetByOffset<Type>(raw, offset);
			return _GetHashCode(item, offset);
		}

		template<typename Type>
		static size_t _GetHashCode(const Type& item, size_t /*offset*/)
		{
			return DataTraits::GetHashCode(item);	//?
		}

		template<typename Void, typename Type, typename... Types>
		bool _IsEqual(const Raw* raw1, const Raw* raw2, const size_t* offsets) const
		{
			const Type& item1 = mColumnList->template GetByOffset<Type>(raw1, *offsets);
			const Type& item2 = mColumnList->template GetByOffset<Type>(raw2, *offsets);
			return DataTraits::IsEqual(item1, item2)
				&& _IsEqual<void, Types...>(raw1, raw2, offsets + 1);
		}

		template<typename Void>
		bool _IsEqual(const Raw* /*raw1*/, const Raw* /*raw2*/, const size_t* /*offsets*/) const noexcept
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

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
		typedef std::function<size_t(const Raw*, size_t*)> HashFunc;
		typedef std::function<bool(const Raw*, const Raw*)> EqualFunc;

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

		class HashTraits : public momo::HashTraits<HashRawKey>
		{
		public:
			typedef std::function<bool(const Raw*, const Raw*)> EqualFunc;

			template<typename KeyArg>
			struct IsValidKeyArg : public std::false_type
			{
			};

			template<typename... Types>
			struct IsValidKeyArg<HashTupleKey<Types...>> : public std::true_type
			{
			};

		public:
			explicit HashTraits(EqualFunc equalFunc)
				: mEqualFunc(equalFunc)
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

		typedef momo::internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef Array<size_t, MemManagerPtr> Offsets;

		typedef momo::HashSet<HashRawKey, HashTraits, MemManagerPtr> HashSet;

		struct UniqueHash
		{
			HashSet hashSet;
			HashFunc hashFunc;
			Offsets sortedOffsets;
		};

		typedef momo::HashMultiMap<HashRawKey, Raw*, HashTraits, MemManagerPtr> HashMultiMap;

		struct MultiHash
		{
			HashMultiMap hashMultiMap;
			HashFunc hashFunc;
			Offsets sortedOffsets;
		};

		typedef Array<UniqueHash, MemManagerPtr> UniqueHashes;

		typedef Array<MultiHash, MemManagerPtr> MultiHashes;

		struct UniqueHashBounds
		{
		public:
			typedef Raw* const* Iterator;

		public:
			explicit UniqueHashBounds(Raw* raw) noexcept
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

			//MOMO_FRIENDS_BEGIN_END(const UniqueHashBounds&, Iterator)

		private:
			Raw* mRaws[1];
		};

		typedef typename HashMultiMap::ConstValueBounds MultiHashBounds;

		typedef typename HashSet::ConstIterator HashSetIterator;
		typedef Array<HashSetIterator, MemManagerPtr,
			ArrayItemTraits<HashSetIterator>, ArraySettings<8>> HashSetIterators;

		typedef typename HashMultiMap::Iterator HashMultiMapIterator;
		typedef Array<HashMultiMapIterator, MemManagerPtr,
			ArrayItemTraits<HashMultiMapIterator>, ArraySettings<8>> HashMultiMapIterators;

		typedef Array<bool, MemManagerPtr> OffsetMarks;
		typedef Array<size_t, MemManagerPtr> OffsetHashCodes;

	public:
		DataIndexes(const ColumnList* columnList, MemManager& memManager)
			: mColumnList(columnList),
			mUniqueHashes(MemManagerPtr(memManager)),
			mMultiHashes(MemManagerPtr(memManager)),
			mOffsetMarks(columnList->GetTotalSize(), false, MemManagerPtr(memManager)),
			mOffsetHashCodes(OffsetHashCodes::CreateCap(
				columnList->GetTotalSize(), MemManagerPtr(memManager)))
		{
		}

		DataIndexes(DataIndexes&& indexes) noexcept
			: mColumnList(indexes.mColumnList),
			mUniqueHashes(std::move(indexes.mUniqueHashes)),
			mMultiHashes(std::move(indexes.mMultiHashes)),
			mOffsetMarks(std::move(indexes.mOffsetMarks)),
			mOffsetHashCodes(std::move(indexes.mOffsetHashCodes))
		{
		}

		DataIndexes(const DataIndexes&) = delete;

		~DataIndexes() noexcept
		{
		}

		DataIndexes& operator=(DataIndexes&& indexes)
		{
			DataIndexes(std::move(indexes)).Swap(*this);
		}

		DataIndexes& operator=(const DataIndexes&) = delete;

		void Swap(DataIndexes& indexes) noexcept
		{
			std::swap(mColumnList, indexes.mColumnList);
			mUniqueHashes.Swap(indexes.mUniqueHashes);
			mMultiHashes.Swap(indexes.mMultiHashes);
			mOffsetMarks.Swap(indexes.mOffsetMarks);
			mOffsetHashCodes.Swap(indexes.mOffsetHashCodes);
		}

		template<typename Raws, typename... Types>
		bool AddUniqueHash(const Raws& raws, const Column<Types>&... columns)
		{
			return _AddIndex<HashSet>(mUniqueHashes, raws, columns...);
		}

		template<typename Raws, typename... Types>
		bool AddMultiHash(const Raws& raws, const Column<Types>&... columns)
		{
			return _AddIndex<HashMultiMap>(mMultiHashes, raws, columns...);
		}

		void AddRaw(Raw* raw)
		{
			size_t uniqueHashCount = mUniqueHashes.GetCount();
			HashSetIterators uniqueHashIters(uniqueHashCount, _GetMemManagerPtr());
			size_t uniqueHashIndex = 0;
			size_t multiHashCount = mMultiHashes.GetCount();
			HashMultiMapIterators multiHashIters(multiHashCount, _GetMemManagerPtr());
			size_t multiHashIndex = 0;
			std::fill(mOffsetHashCodes.GetBegin(), mOffsetHashCodes.GetEnd(), 0);
			size_t* hashCodes = mOffsetHashCodes.IsEmpty() ? nullptr : mOffsetHashCodes.GetItems();
			try
			{
				for (; uniqueHashIndex < uniqueHashCount; ++uniqueHashIndex)
				{
					UniqueHash& uniqueHash = mUniqueHashes[uniqueHashIndex];
					uniqueHashIters[uniqueHashIndex] = _AddRaw(uniqueHash.hashSet, raw,
						uniqueHash.hashFunc(raw, hashCodes));
				}
				for (; multiHashIndex < multiHashCount; ++multiHashIndex)
				{
					MultiHash& multiHash = mMultiHashes[multiHashIndex];
					multiHashIters[multiHashIndex] = _AddRaw(multiHash.hashMultiMap, raw,
						multiHash.hashFunc(raw, hashCodes));
				}
			}
			catch (...)
			{
				for (size_t i = 0; i < uniqueHashIndex; ++i)
					mUniqueHashes[i].hashSet.Remove(uniqueHashIters[i]);
				for (size_t i = 0; i < multiHashIndex; ++i)
					mMultiHashes[i].hashMultiMap.Remove(multiHashIters[i]);
				throw;
			}
		}

		void Clear() noexcept
		{
			for (UniqueHash& uniqueHash : mUniqueHashes)
				uniqueHash.hashSet.Clear();
			for (MultiHash& multiHash : mMultiHashes)
				multiHash.hashMultiMap.Clear();
		}

		template<size_t columnCount>
		const UniqueHash* FindUniqueHash(const size_t (&sortedOffsets)[columnCount]) const
		{
			for (const UniqueHash& uniqueHash : mUniqueHashes)
			{
				bool includes = std::includes(sortedOffsets, sortedOffsets + columnCount,
					uniqueHash.sortedOffsets.GetBegin(), uniqueHash.sortedOffsets.GetEnd());
				if (includes)
					return &uniqueHash;
			}
			return nullptr;
		}

		template<size_t columnCount>
		const MultiHash* FindMultiHash(const size_t (&sortedOffsets)[columnCount]) const
		{
			const MultiHash* resMultiHash = nullptr;
			size_t maxKeyCount = 0;
			for (const MultiHash& multiHash : mMultiHashes)
			{
				bool includes = std::includes(sortedOffsets, sortedOffsets + columnCount,
					multiHash.sortedOffsets.GetBegin(), multiHash.sortedOffsets.GetEnd());
				size_t keyCount = multiHash.hashMultiMap.GetKeyCount();
				if (includes && keyCount > maxKeyCount)
				{
					maxKeyCount = keyCount;
					resMultiHash = &multiHash;
				}
			}
			return resMultiHash;
		}

		template<typename... Types>
		UniqueHashBounds Find(const UniqueHash& uniqueHash, const OffsetItemTuple<Types...>& key) const
		{
			HashTupleKey<Types...> hashTupleKey{ key, _GetHashCode<0>(key), mColumnList };
			HashSetIterator keyIter = uniqueHash.hashSet.Find(hashTupleKey);
			return UniqueHashBounds(!!keyIter ? keyIter->raw : nullptr);
		}

		template<typename... Types>
		MultiHashBounds Find(const MultiHash& multiHash, const OffsetItemTuple<Types...>& key) const
		{
			HashTupleKey<Types...> hashTupleKey{ key, _GetHashCode<0>(key), mColumnList };
			auto keyIter = multiHash.hashMultiMap.Find(hashTupleKey);
			return !!keyIter ? keyIter->values : MultiHashBounds();
		}

		template<size_t columnCount>
		static void GetSortedOffsets(const size_t (&offsets)[columnCount], size_t* sortedOffsets)
		{
			std::copy_n(offsets, columnCount, sortedOffsets);
			std::sort(sortedOffsets, sortedOffsets + columnCount);
			MOMO_ASSERT(std::unique(sortedOffsets, sortedOffsets + columnCount)
				== sortedOffsets + columnCount);
		}

		template<typename Index>
		static bool HasOffset(const Index& index, size_t offset) noexcept
		{
			return std::binary_search(index.sortedOffsets.GetBegin(),
				index.sortedOffsets.GetEnd(), offset);
		}

		const bool* GetOffsetMarks() const noexcept
		{
			return mOffsetMarks.GetItems();
		}

	private:
		MemManagerPtr _GetMemManagerPtr() const noexcept
		{
			return mOffsetMarks.GetMemManager();
		}

		template<typename HashContainer, typename Indexes, typename Raws, typename... Types>
		bool _AddIndex(Indexes& indexes, const Raws& raws, const Column<Types>&... columns)
		{
			static const size_t columnCount = sizeof...(Types);
			size_t offsets[columnCount] = { mColumnList->GetOffset(columns)... };
			Offsets sortedOffsets(columnCount, _GetMemManagerPtr());
			GetSortedOffsets(offsets, sortedOffsets.GetItems());
			for (const auto& index : indexes)
			{
				bool equal = std::equal(index.sortedOffsets.GetBegin(), index.sortedOffsets.GetEnd(),
					sortedOffsets.GetBegin(), sortedOffsets.GetEnd());
				if (equal)
					return false;
			}
			auto hashFunc = [this, offsets] (const Raw* raw, size_t* hashCodes)
			{
				if (hashCodes == nullptr)
					return _GetHashCode<void, Types...>(raw, offsets);
				else
					return _GetHashCode<void, Types...>(raw, offsets, hashCodes);
			};
			auto equalFunc = [this, offsets] (const Raw* raw1, const Raw* raw2)
				{ return _IsEqual<void, Types...>(raw1, raw2, offsets); };
			indexes.Reserve(indexes.GetCount() + 1);
			HashTraits hashTraits(equalFunc);
			HashContainer hashCont(hashTraits, _GetMemManagerPtr());
			for (Raw* raw : raws)
				_AddRaw(hashCont, raw, hashFunc(raw, nullptr));
			indexes.AddBackNogrow({ std::move(hashCont), hashFunc, std::move(sortedOffsets) });	//?
			for (size_t offset : offsets)
			{
				if (mOffsetMarks[offset])
					mOffsetHashCodes.SetCount(mColumnList->GetTotalSize());
				mOffsetMarks[offset] = true;
			}
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
			return hashCode + _GetHashCode<void, Types...>(raw, offsets + 1, hashCodes);
		}

		template<typename Void>
		size_t _GetHashCode(const Raw* /*raw*/, const size_t* /*offsets*/,
			size_t* /*hashCodes*/) const noexcept
		{
			return 0;
		}

		template<size_t index, typename... Types>
		static size_t _GetHashCode(const OffsetItemTuple<Types...>& key,
			typename std::enable_if<(index < sizeof...(Types)), int>::type = 0)
		{
			const auto& pair = std::get<index>(key);
			const auto& item = pair.second;
			return _GetHashCode(item, pair.first) + _GetHashCode<index + 1>(key);
		}

		template<size_t index, typename... Types>
		static size_t _GetHashCode(const OffsetItemTuple<Types...>& /*key*/,
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
			return DataTraits::GetHashCode(item);
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

		static HashSetIterator _AddRaw(HashSet& hashSet, Raw* raw, size_t hashCode)
		{
			auto insRes = hashSet.Insert({ raw, hashCode });
			if (!insRes.inserted)
				throw std::runtime_error("Unique index violation");
			return insRes.iterator;
		}

		static HashMultiMapIterator _AddRaw(HashMultiMap& hashMultiMap, Raw* raw, size_t hashCode)
		{
			return hashMultiMap.Add({ raw, hashCode }, raw);
		}

	private:
		const ColumnList* mColumnList;
		UniqueHashes mUniqueHashes;
		MultiHashes mMultiHashes;
		OffsetMarks mOffsetMarks;
		OffsetHashCodes mOffsetHashCodes;
	};
}

} // namespace experimental

} // namespace momo

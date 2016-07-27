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

		typedef momo::internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef Array<size_t, MemManagerPtr> Offsets;

		class UniqueHash
		{
		private:
			typedef momo::HashSet<HashRawKey, HashTraits, MemManagerPtr> HashSet;

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
				Raw* mRaws[1];
			};

		public:
			UniqueHash(Offsets&& sortedOffsets, HashFunc&& hashFunc, EqualFunc&& equalFunc) noexcept
				: mSortedOffsets(std::move(sortedOffsets)),
				mHashFunc(std::move(hashFunc)),
				mHashSet(HashTraits(std::move(equalFunc)), MemManagerPtr(mSortedOffsets.GetMemManager()))
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

		//typedef typename UniqueHash::RawBounds UniqueHashBounds;

		typedef Array<UniqueHash, MemManagerPtr> UniqueHashes;

		typedef typename UniqueHash::Iterator UniqueHashIterator;
		typedef Array<UniqueHashIterator, MemManagerPtr,
			ArrayItemTraits<UniqueHashIterator>, ArraySettings<8>> UniqueHashIterators;

		class MultiHash
		{
		private:
			typedef momo::HashMultiMap<HashRawKey, Raw*, HashTraits, MemManagerPtr> HashMultiMap;
			typedef momo::HashMap<Raw*, size_t, momo::HashTraits<Raw*>, MemManagerPtr> HashMap;

		public:
			typedef typename HashMultiMap::Iterator Iterator;

			typedef typename HashMultiMap::ConstValueBounds RawBounds;

		public:
			MultiHash(Offsets&& sortedOffsets, HashFunc&& hashFunc, EqualFunc&& equalFunc) noexcept
				: mSortedOffsets(std::move(sortedOffsets)),
				mHashFunc(std::move(hashFunc)),
				mHashMultiMap(HashTraits(std::move(equalFunc)), MemManagerPtr(mSortedOffsets.GetMemManager())),
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
				try
				{
					mHashMap.Insert(raw,
						iter.GetValuePtr() - iter.GetKeyIterator()->values.GetBegin());
				}
				catch (...)
				{
					mHashMultiMap.Remove(iter);
					throw;
				}
				return iter;
			}

			void Remove(Iterator iter) noexcept
			{
				mHashMap.Remove(iter->value);
				mHashMultiMap.Remove(iter);
			}

		private:
			Offsets mSortedOffsets;
			HashFunc mHashFunc;
			HashMultiMap mHashMultiMap;
			HashMap mHashMap;
		};

		//typedef typename MultiHash::RawBounds MultiHashBounds;

		typedef Array<MultiHash, MemManagerPtr> MultiHashes;

		typedef typename MultiHash::Iterator MultiHashIterator;
		typedef Array<MultiHashIterator, MemManagerPtr,
			ArrayItemTraits<MultiHashIterator>, ArraySettings<8>> MultiHashIterators;

		typedef Array<bool, MemManagerPtr> OffsetMarks;
		typedef Array<size_t, MemManagerPtr> OffsetHashCodes;

	public:
		DataIndexes(const ColumnList* columnList, MemManager& memManager)
			: mColumnList(columnList),
			mUniqueHashes(MemManagerPtr(memManager)),
			mMultiHashes(MemManagerPtr(memManager)),
			mOffsetMarks(columnList->GetTotalSize(), true, MemManagerPtr(memManager)),
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
			mOffsetMarks.Swap(indexes.mOffsetMarks);
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
		typename Hash::RawBounds FindRaws(const Hash& hash, const OffsetItemTuple<Types...>& key) const
		{
			HashTupleKey<Types...> hashTupleKey{ key, _GetHashCode<0>(key), mColumnList };
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
					mUniqueHashes[uniqueHashIndex].Add(raw, hashCodes);
				for (; multiHashIndex < multiHashCount; ++multiHashIndex)
					mMultiHashes[multiHashIndex].Add(raw, hashCodes);
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

		template<size_t columnCount>
		const UniqueHash* FindFitUniqueHash(const size_t (&sortedOffsets)[columnCount]) const noexcept
		{
			for (const UniqueHash& uniqueHash : mUniqueHashes)
			{
				const Offsets& curSortedOffsets = uniqueHash.GetSortedOffsets();
				bool includes = std::includes(sortedOffsets, sortedOffsets + columnCount,
					curSortedOffsets.GetBegin(), curSortedOffsets.GetEnd());
				if (includes)
					return &uniqueHash;
			}
			return nullptr;
		}

		template<size_t columnCount>
		const MultiHash* FindFitMultiHash(const size_t (&sortedOffsets)[columnCount]) const noexcept
		{
			const MultiHash* resMultiHash = nullptr;
			size_t maxKeyCount = 0;
			for (const MultiHash& multiHash : mMultiHashes)
			{
				const Offsets& curSortedOffsets = multiHash.GetSortedOffsets();
				bool includes = std::includes(sortedOffsets, sortedOffsets + columnCount,
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
		static void GetSortedOffsets(const size_t (&offsets)[columnCount], size_t* sortedOffsets)
		{
			std::copy_n(offsets, columnCount, sortedOffsets);
			std::sort(sortedOffsets, sortedOffsets + columnCount);
			MOMO_ASSERT(std::unique(sortedOffsets, sortedOffsets + columnCount)
				== sortedOffsets + columnCount);
		}

		template<typename Hash>
		static bool HasOffset(const Hash& hash, size_t offset) noexcept
		{
			const Offsets& sortedOffsets = hash.GetSortedOffsets();
			return std::binary_search(sortedOffsets.GetBegin(), sortedOffsets.GetEnd(), offset);
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

		template<typename Hashes, typename... Types>
		const typename Hashes::Item* _FindHash(const Hashes& hashes, const Column<Types>&... columns) const
		{
			static const size_t columnCount = sizeof...(Types);
			size_t offsets[columnCount] = { mColumnList->GetOffset(columns)... };
			size_t sortedOffsets[columnCount];
			return _FindHash(hashes, offsets, sortedOffsets, columns...);
		}

		template<typename Hashes, size_t columnCount, typename... Types>
		const typename Hashes::Item* _FindHash(const Hashes& hashes, const size_t (&offsets)[columnCount],
			size_t* sortedOffsets, const Column<Types>&... columns) const
		{
			GetSortedOffsets(offsets, sortedOffsets);
			for (const auto& hash : hashes)
			{
				const Offsets& curSortedOffsets = hash.GetSortedOffsets();
				bool equal = std::equal(curSortedOffsets.GetBegin(), curSortedOffsets.GetEnd(),
					sortedOffsets, sortedOffsets + columnCount);
				if (equal)
					return &hash;
			}
			return nullptr;
		}

		template<typename Hashes, typename Raws, typename... Types>
		bool _AddHash(Hashes& hashes, const Raws& raws, const Column<Types>&... columns)
		{
			static const size_t columnCount = sizeof...(Types);
			size_t offsets[columnCount] = { mColumnList->GetOffset(columns)... };
			Offsets sortedOffsets(columnCount, _GetMemManagerPtr());
			if (_FindHash(hashes, offsets, sortedOffsets.GetItems(), columns...) != nullptr)
				return false;
			auto hashFunc = [this, offsets] (const Raw* raw, size_t* hashCodes)
			{
				if (hashCodes == nullptr)
					return _GetHashCode<void, Types...>(raw, offsets);
				else
					return _GetHashCode<void, Types...>(raw, offsets, hashCodes);
			};
			auto equalFunc = [this, offsets] (const Raw* raw1, const Raw* raw2)
				{ return _IsEqual<void, Types...>(raw1, raw2, offsets); };
			typename Hashes::Item hash(std::move(sortedOffsets), hashFunc, equalFunc);
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
		static size_t _GetHashCode(const OffsetItemTuple<Types...>& key,
			typename std::enable_if<(index < sizeof...(Types)), int>::type = 0)
		{
			const auto& pair = std::get<index>(key);
			const auto& item = pair.second;
			return _GetHashCode(item, pair.first) + _GetHashCode<index + 1>(key);	//?
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
		OffsetMarks mOffsetMarks;
		OffsetHashCodes mOffsetHashCodes;
	};
}

} // namespace experimental

} // namespace momo

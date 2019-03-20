/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/DataIndexes.h

\**********************************************************/

#pragma once

#include "HashMultiMap.h"

namespace momo
{

namespace internal
{
	template<typename TColumnList>
	class DataRawUniqueHashIterator
	{
	protected:
		typedef TColumnList ColumnList;

	public:
		typedef typename ColumnList::Raw Raw;

		typedef Raw* const& Reference;
		typedef Raw* const* Pointer;

		typedef DataRawUniqueHashIterator ConstIterator;

	private:
		typedef typename ColumnList::Settings Settings;

	public:
		explicit DataRawUniqueHashIterator() noexcept
			: mRaw(nullptr),
			mRawIndex(0)
		{
		}

		explicit DataRawUniqueHashIterator(Raw* raw, size_t rawIndex) noexcept
			: mRaw(raw),
			mRawIndex(rawIndex)
		{
			//MOMO_ASSERT(rawIndex <= (size_t)((raw != nullptr) ? 1 : 0));
		}

		DataRawUniqueHashIterator& operator+=(ptrdiff_t diff)
		{
			size_t newRawIndex = mRawIndex + diff;
			MOMO_CHECK(mRawIndex <= (size_t)((mRaw != nullptr) ? 1 : 0));
			mRawIndex = newRawIndex;
			return *this;
		}

		ptrdiff_t operator-(ConstIterator iter) const
		{
			MOMO_CHECK(mRaw == iter.mRaw);
			return mRawIndex - iter.mRawIndex;
		}

		Pointer operator->() const
		{
			MOMO_CHECK(mRaw != nullptr && mRawIndex == 0);
			return &mRaw;
		}

		bool operator==(ConstIterator iter) const noexcept
		{
			return mRaw == iter.mRaw && mRawIndex == iter.mRawIndex;
		}

		bool operator<(ConstIterator iter) const
		{
			MOMO_CHECK(mRaw == iter.mRaw);
			return mRawIndex < iter.mRawIndex;
		}

		MOMO_MORE_ARRAY_ITERATOR_OPERATORS(DataRawUniqueHashIterator)

	private:
		Raw* mRaw;
		size_t mRawIndex;
	};

	template<typename TColumnList, typename TKeyIterator>
	class DataRawMultiHashIterator : private VersionKeeper<typename TColumnList::Settings>
	{
	protected:
		typedef TColumnList ColumnList;

	public:
		typedef TKeyIterator KeyIterator;

	private:
		typedef typename ColumnList::Settings Settings;
		typedef typename ColumnList::Raw Raw;

	public:
		typedef Raw* const& Reference;
		typedef Raw* const* Pointer;

		typedef DataRawMultiHashIterator ConstIterator;

		typedef internal::VersionKeeper<Settings> VersionKeeper;

	public:
		explicit DataRawMultiHashIterator() noexcept
			: mRawIndex(0)
		{
		}

		explicit DataRawMultiHashIterator(KeyIterator keyIter, size_t rawIndex,
			VersionKeeper version) noexcept
			: VersionKeeper(version),
			mKeyIterator(keyIter),
			mRawIndex(rawIndex)
		{
			//MOMO_ASSERT(rawIndex <= (!!keyIter ? keyIter->values.GetCount() + 1 : 0));
		}

		DataRawMultiHashIterator& operator+=(ptrdiff_t diff)
		{
			if (diff != 0)
			{
				VersionKeeper::Check();
				size_t newRawIndex = mRawIndex + diff;
				MOMO_CHECK(!!mKeyIterator && newRawIndex <= mKeyIterator->values.GetCount() + 1);
				mRawIndex = newRawIndex;
			}
			return *this;
		}

		ptrdiff_t operator-(ConstIterator iter) const
		{
			MOMO_CHECK(mKeyIterator == iter.mKeyIterator);
			return mRawIndex - iter.mRawIndex;
		}

		Pointer operator->() const
		{
			VersionKeeper::Check();
			MOMO_CHECK(!!mKeyIterator && mRawIndex <= mKeyIterator->values.GetCount());
			if (mRawIndex > 0)
				return &mKeyIterator->values[mRawIndex - 1];
			else
				return &mKeyIterator->key;
		}

		bool operator==(ConstIterator iter) const noexcept
		{
			return mKeyIterator == iter.mKeyIterator
				&& mRawIndex == iter.mRawIndex;
		}

		bool operator<(ConstIterator iter) const
		{
			MOMO_CHECK(mKeyIterator == iter.mKeyIterator);
			return mRawIndex < iter.mRawIndex;
		}

		MOMO_MORE_ARRAY_ITERATOR_OPERATORS(DataRawMultiHashIterator)

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

		template<typename Item>
		using Column = typename ColumnList::template Column<Item>;

		template<typename... Items>
		using OffsetItemTuple = std::tuple<std::pair<size_t, const Items&>...>;

	private:
		typedef internal::VersionKeeper<Settings> VersionKeeper;

		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef NestedArrayIntCap<4, size_t, MemManagerPtr> Offsets;

		template<typename... Items>
		struct HashTupleKey
		{
			OffsetItemTuple<Items...> tuple;
			size_t hashCode;
		};

		template<typename Item = void>
		struct HashMixedKey
		{
			Raw* raw;
			size_t offset;
			const Item* item;
		};

		typedef std::function<size_t(Raw*, const size_t*)> HashFunc;
		typedef std::function<bool(Raw*, Raw*, const size_t*)> EqualFunc;
		typedef std::function<size_t(HashMixedKey<>, const size_t*)> HashMixedFunc;
		typedef std::function<bool(HashMixedKey<>, Raw*, const size_t*)> EqualMixedFunc;

		class HashTraits : public momo::HashTraits<Raw*, typename DataTraits::HashBucket>
		{
		public:
			template<typename KeyArg>
			struct IsValidKeyArg : public std::false_type
			{
			};

			template<typename... Items>
			struct IsValidKeyArg<HashTupleKey<Items...>> : public std::true_type
			{
			};

			template<typename Item>
			struct IsValidKeyArg<HashMixedKey<Item>> : public std::true_type
			{
			};

			static const bool isFastNothrowHashable = false;

		public:
			explicit HashTraits(HashFunc&& hashFunc, EqualFunc&& equalFunc,
				HashMixedFunc&& hashMixedFunc, EqualMixedFunc&& equalMixedFunc,
				Offsets&& offsets) noexcept
				: mHashFunc(std::move(hashFunc)),
				mEqualFunc(std::move(equalFunc)),
				mHashMixedFunc(std::move(hashMixedFunc)),
				mEqualMixedFunc(std::move(equalMixedFunc)),
				mOffsets(std::move(offsets))
			{
			}

			explicit HashTraits(const HashTraits& hashTraits, MemManagerPtr&& memManagerPtr)
				: mHashFunc(hashTraits.mHashFunc),
				mEqualFunc(hashTraits.mEqualFunc),
				mHashMixedFunc(hashTraits.mHashMixedFunc),
				mEqualMixedFunc(hashTraits.mEqualMixedFunc),
				mOffsets(hashTraits.mOffsets, std::move(memManagerPtr))
			{
			}

			size_t GetHashCode(Raw* key) const
			{
				return mHashFunc(key, mOffsets.GetItems());
			}

			template<typename... Items>
			size_t GetHashCode(const HashTupleKey<Items...>& key) const noexcept
			{
				return key.hashCode;
			}

			template<typename Item>
			size_t GetHashCode(const HashMixedKey<Item>& key) const
			{
				return mHashMixedFunc({ key.raw, key.offset, key.item }, mOffsets.GetItems());
			}

			bool IsEqual(Raw* key1, Raw* key2) const
			{
				//if (key1 == key2)
				//	return true;
				return mEqualFunc(key1, key2, mOffsets.GetItems());
			}

			template<typename... Items>
			bool IsEqual(const HashTupleKey<Items...>& key1, Raw* key2) const
			{
				return pvIsEqual<0>(key1, key2);
			}

			template<typename Item>
			bool IsEqual(const HashMixedKey<Item>& key1, Raw* key2) const
			{
				return mEqualMixedFunc({ key1.raw, key1.offset, key1.item }, key2,
					mOffsets.GetItems());
			}

		private:
			template<size_t index, typename... Items>
			EnableIf<(index < sizeof...(Items)), bool> pvIsEqual(
				const HashTupleKey<Items...>& key1, Raw* key2) const
			{
				const auto& pair = std::get<index>(key1.tuple);
				const auto& item1 = pair.second;
				typedef typename std::decay<decltype(item1)>::type Item;
				const Item& item2 = ColumnList::template GetByOffset<const Item>(key2, pair.first);
				return DataTraits::IsEqual(item1, item2) && pvIsEqual<index + 1>(key1, key2);
			}

			template<size_t index, typename... Items>
			EnableIf<(index == sizeof...(Items)), bool> pvIsEqual(
				const HashTupleKey<Items...>& /*key1*/, Raw* /*key2*/) const noexcept
			{
				return true;
			}

		private:
			HashFunc mHashFunc;
			EqualFunc mEqualFunc;
			HashMixedFunc mHashMixedFunc;
			EqualMixedFunc mEqualMixedFunc;
			Offsets mOffsets;
		};

	public:
		class UniqueHash;

		class UniqueIndexViolation : public std::runtime_error
		{
		public:
			explicit UniqueIndexViolation(Raw* raw, const UniqueHash& uniqueHash)
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
			typedef momo::HashSet<Raw*, HashTraits, MemManagerPtr,
				HashSetItemTraits<Raw*, Raw*, MemManagerPtr>, NestedHashSetSettings> HashSet;

			typedef typename HashSet::ConstIterator Iterator;

		public:
			class RawBounds
			{
			public:
				typedef DataRawUniqueHashIterator<ColumnList> Iterator;

				typedef RawBounds ConstBounds;

			public:
				explicit RawBounds(Raw* raw) noexcept
					: mRaw(raw)
				{
				}

				Iterator GetBegin() const noexcept
				{
					return Iterator(mRaw, 0);
				}

				Iterator GetEnd() const noexcept
				{
					return Iterator(mRaw, GetCount());
				}

				MOMO_FRIENDS_BEGIN_END(const RawBounds&, Iterator)

				size_t GetCount() const noexcept
				{
					return (mRaw != nullptr) ? 1 : 0;
				}

			private:
				Raw* mRaw;
			};

		public:
			explicit UniqueHash(HashTraits&& hashTraits, Offsets&& sortedOffsets)
				: mSortedOffsets(std::move(sortedOffsets)),
				mHashSet(std::move(hashTraits), MemManagerPtr(mSortedOffsets.GetMemManager()))
			{
			}

			UniqueHash(UniqueHash&& uniqueHash) noexcept
				: mSortedOffsets(std::move(uniqueHash.mSortedOffsets)),
				mHashSet(std::move(uniqueHash.mHashSet)),
				mIteratorAdd(uniqueHash.mIteratorAdd),
				mIteratorRemove(uniqueHash.mIteratorRemove)
			{
			}

			UniqueHash(const UniqueHash&) = delete;

			~UniqueHash() noexcept
			{
			}

			UniqueHash& operator=(UniqueHash&& uniqueHash) noexcept
			{
				mSortedOffsets = std::move(uniqueHash.mSortedOffsets);
				mHashSet = std::move(uniqueHash.mHashSet);
				mIteratorAdd = uniqueHash.mIteratorAdd;
				mIteratorRemove = uniqueHash.mIteratorRemove;
				return *this;
			}

			UniqueHash& operator=(const UniqueHash&) = delete;

			const Offsets& GetSortedOffsets() const noexcept
			{
				return mSortedOffsets;
			}

			const HashTraits& GetHashTraits() const noexcept
			{
				return mHashSet.GetHashTraits();
			}

			void Clear() noexcept
			{
				mHashSet.Clear();
			}

			void Reserve(size_t capacity)
			{
				mHashSet.Reserve(capacity);
			}

			RawBounds Find(Raw* raw, VersionKeeper /*version*/) const
			{
				return pvFind(raw);
			}

			template<typename... Items>
			RawBounds Find(const HashTupleKey<Items...>& hashTupleKey, VersionKeeper /*version*/) const
			{
				return pvFind(hashTupleKey);
			}

			bool Add(Raw* raw, Raw* oldRaw = nullptr)
			{
				MOMO_ASSERT(!mIteratorAdd);
				auto insRes = mHashSet.Insert(raw);
				if (!insRes.inserted && *insRes.iterator != oldRaw)
					throw UniqueIndexViolation(*insRes.iterator, *this);
				mIteratorAdd = insRes.iterator;
				return *mIteratorAdd == raw;
			}

			template<typename Item>
			void Add(HashMixedKey<Item> hashMixedKey)
			{
				MOMO_ASSERT(!mIteratorAdd);
				Iterator iter = mHashSet.Find(hashMixedKey);
				if (!!iter)
					throw UniqueIndexViolation(*iter, *this);
				mIteratorAdd = mHashSet.Add(iter, hashMixedKey.raw);
			}

			void RejectAdd() noexcept
			{
				if (!!mIteratorAdd)
					mHashSet.Remove(mIteratorAdd);
				mIteratorAdd = Iterator();
			}

			void RejectAdd(Raw* raw) noexcept
			{
				if (!!mIteratorAdd && *mIteratorAdd == raw)
					mHashSet.Remove(mIteratorAdd);
				mIteratorAdd = Iterator();
			}

			void AcceptAdd() noexcept
			{
				mIteratorAdd = Iterator();
			}

			void AcceptAdd(Raw* raw) noexcept
			{
				if (!!mIteratorAdd)
					mHashSet.ResetKey(mIteratorAdd, raw);
				mIteratorAdd = Iterator();
			}

			void PrepareRemove(Raw* raw)
			{
				MOMO_ASSERT(!mIteratorRemove);
				mIteratorRemove = mHashSet.Find(raw);
				MOMO_ASSERT(!!mIteratorRemove);
			}

			void RejectRemove() noexcept
			{
				mIteratorRemove = Iterator();
			}

			void AcceptRemove() noexcept
			{
				if (!!mIteratorRemove)
					mHashSet.Remove(mIteratorRemove);
				mIteratorRemove = Iterator();
			}

			template<typename RawFilter>
			void FilterRaws(RawFilter rawFilter) noexcept
			{
				Iterator iter = mHashSet.GetBegin();
				while (!!iter)
				{
					if (rawFilter(*iter))
						++iter;
					else
						iter = mHashSet.Remove(iter);
				}
			}

		private:
			template<typename Key>
			RawBounds pvFind(const Key& key) const
			{
				Iterator iter = mHashSet.Find(key);
				return RawBounds(!!iter ? *iter : nullptr);
			}

		private:
			Offsets mSortedOffsets;
			HashSet mHashSet;
			Iterator mIteratorAdd;
			Iterator mIteratorRemove;
		};

		class MultiHash
		{
		private:
			typedef momo::HashMultiMap<Raw*, Raw*, HashTraits, MemManagerPtr,
				HashMultiMapKeyValueTraits<Raw*, Raw*, MemManagerPtr>,
				NestedHashMultiMapSettings> HashMultiMap;

			typedef typename HashMultiMap::ConstKeyIterator KeyIterator;

		public:
			class RawBounds : private VersionKeeper
			{
			public:
				typedef DataRawMultiHashIterator<ColumnList, KeyIterator> Iterator;

				typedef RawBounds ConstBounds;

			public:
				explicit RawBounds(KeyIterator keyIter, VersionKeeper version) noexcept
					: VersionKeeper(version),
					mKeyIterator(keyIter),
					mRawCount(!!keyIter ? keyIter->values.GetCount() + 1 : 0)
				{
				}

				Iterator GetBegin() const noexcept
				{
					return Iterator(mKeyIterator, 0, *this);
				}

				Iterator GetEnd() const noexcept
				{
					return Iterator(mKeyIterator, mRawCount, *this);
				}

				MOMO_FRIENDS_BEGIN_END(const RawBounds&, Iterator)

				size_t GetCount() const noexcept
				{
					return mRawCount;
				}

			private:
				KeyIterator mKeyIterator;
				size_t mRawCount;
			};

		public:
			explicit MultiHash(HashTraits&& hashTraits, Offsets&& sortedOffsets)
				: mSortedOffsets(std::move(sortedOffsets)),
				mHashMultiMap(std::move(hashTraits), MemManagerPtr(mSortedOffsets.GetMemManager()))
			{
			}

			MultiHash(MultiHash&& multiHash) noexcept
				: mSortedOffsets(std::move(multiHash.mSortedOffsets)),
				mHashMultiMap(std::move(multiHash.mHashMultiMap)),
				mKeyIteratorAdd(multiHash.mKeyIteratorAdd),
				mKeyIteratorRemove(multiHash.mKeyIteratorRemove)
			{
			}

			MultiHash(const MultiHash&) = delete;

			~MultiHash() noexcept
			{
			}

			MultiHash& operator=(MultiHash&& multiHash) noexcept
			{
				mSortedOffsets = std::move(multiHash.mSortedOffsets);
				mHashMultiMap = std::move(multiHash.mHashMultiMap);
				mKeyIteratorAdd = multiHash.mKeyIteratorAdd;
				mKeyIteratorRemove = multiHash.mKeyIteratorRemove;
				return *this;
			}

			MultiHash& operator=(const MultiHash&) = delete;

			const Offsets& GetSortedOffsets() const noexcept
			{
				return mSortedOffsets;
			}

			const HashTraits& GetHashTraits() const noexcept
			{
				return mHashMultiMap.GetHashTraits();
			}

			size_t GetKeyCount() const noexcept
			{
				return mHashMultiMap.GetKeyCount();
			}

			void Clear() noexcept
			{
				mHashMultiMap.Clear();
			}

			template<typename... Items>
			RawBounds Find(const HashTupleKey<Items...>& hashTupleKey, VersionKeeper version) const
			{
				return RawBounds(mHashMultiMap.Find(hashTupleKey), version);
			}

			void Add(Raw* raw)
			{
				MOMO_ASSERT(!mKeyIteratorAdd);
				KeyIterator keyIter = mHashMultiMap.InsertKey(raw);
				if (keyIter->key != raw)
					mHashMultiMap.Add(keyIter, raw);
				mKeyIteratorAdd = keyIter;
			}

			template<typename Item>
			void Add(HashMixedKey<Item> hashMixedKey)
			{
				MOMO_ASSERT(!mKeyIteratorAdd);
				KeyIterator keyIter = mHashMultiMap.Find(hashMixedKey);
				if (!!keyIter)
				{
					mHashMultiMap.Add(keyIter, hashMixedKey.raw);
					mKeyIteratorAdd = keyIter;
				}
				else
				{
					mKeyIteratorAdd = mHashMultiMap.AddKey(keyIter, hashMixedKey.raw);
				}
			}

			void RejectAdd() noexcept
			{
				if (!mKeyIteratorAdd)
					return;
				size_t valueCount = mKeyIteratorAdd->values.GetCount();
				if (valueCount > 0)
					mHashMultiMap.Remove(mKeyIteratorAdd, valueCount - 1);
				else
					mHashMultiMap.RemoveKey(mKeyIteratorAdd);
				mKeyIteratorAdd = KeyIterator();
			}

			void AcceptAdd() noexcept
			{
				mKeyIteratorAdd = KeyIterator();
			}

			void PrepareRemove(Raw* raw)
			{
				MOMO_ASSERT(!mKeyIteratorRemove);
				mKeyIteratorRemove = mHashMultiMap.Find(raw);
				MOMO_ASSERT(!!mKeyIteratorRemove);
			}

			void RejectRemove() noexcept
			{
				mKeyIteratorRemove = KeyIterator();
			}

			void AcceptRemove(Raw* raw) noexcept
			{
				if (!mKeyIteratorRemove)
					return;
				auto raws = mKeyIteratorRemove->values;
				size_t valueCount = raws.GetCount();
				if (valueCount == 0)
				{
					MOMO_ASSERT(mKeyIteratorRemove->key == raw);
					mHashMultiMap.RemoveKey(mKeyIteratorRemove);
				}
				else if (mKeyIteratorRemove->key == raw)
				{
					mHashMultiMap.ResetKey(mKeyIteratorRemove, raws[valueCount - 1]);
					mHashMultiMap.Remove(mKeyIteratorRemove, valueCount - 1);
				}
				else
				{
					Raw* const* rawPtr = std::find(raws.GetBegin(), raws.GetEnd(), raw);	//?
					mHashMultiMap.Remove(mKeyIteratorRemove, rawPtr - raws.GetBegin());
				}
				mKeyIteratorRemove = KeyIterator();
			}

			template<typename RawFilter>
			void FilterRaws(RawFilter rawFilter) noexcept
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
					if (rawFilter(keyIter->key))
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
					mHashMultiMap.ResetKey(keyIter, keyIter->values[valueCount - 1]);
					mHashMultiMap.Remove(keyIter, valueCount - 1);
					++keyIter;
				}
			}

		private:
			Offsets mSortedOffsets;
			HashMultiMap mHashMultiMap;
			KeyIterator mKeyIteratorAdd;
			KeyIterator mKeyIteratorRemove;
		};

	private:
		template<typename Index>
		using Indexes = Array<Index, MemManagerPtr, ArrayItemTraits<Index, MemManagerPtr>,
			NestedArraySettings<ArraySettings<0, false>>>;	//?

		typedef Indexes<UniqueHash> UniqueHashes;
		typedef Indexes<MultiHash> MultiHashes;

	public:
		explicit DataIndexes(MemManager& memManager) noexcept
			: mUniqueHashes(MemManagerPtr(memManager)),
			mMultiHashes(MemManagerPtr(memManager))
		{
		}

		DataIndexes(DataIndexes&& indexes) noexcept
			: mUniqueHashes(std::move(indexes.mUniqueHashes)),
			mMultiHashes(std::move(indexes.mMultiHashes))
		{
		}

		DataIndexes(const DataIndexes&) = delete;

		~DataIndexes() noexcept
		{
		}

		DataIndexes& operator=(const DataIndexes&) = delete;

		void Swap(DataIndexes& indexes) noexcept
		{
			mUniqueHashes.Swap(indexes.mUniqueHashes);
			mMultiHashes.Swap(indexes.mMultiHashes);
		}

		template<typename... Items>
		const UniqueHash* GetUniqueHash(const ColumnList* columnList,
			const Column<Items>&... columns) const
		{
			return pvGetHash(columnList, mUniqueHashes, columns...);
		}

		template<typename... Items>
		const MultiHash* GetMultiHash(const ColumnList* columnList,
			const Column<Items>&... columns) const
		{
			return pvGetHash(columnList, mMultiHashes, columns...);
		}

		template<typename Raws, typename... Items>
		bool AddUniqueHash(const ColumnList* columnList, const Raws& raws,
			const Column<Items>&... columns)
		{
			return pvAddHash(columnList, mUniqueHashes, raws, columns...);
		}

		template<typename Raws, typename... Items>
		bool AddMultiHash(const ColumnList* columnList, const Raws& raws,
			const Column<Items>&... columns)
		{
			return pvAddHash(columnList, mMultiHashes, raws, columns...);
		}

		template<typename... Items>
		bool RemoveUniqueHash(const ColumnList* columnList, const Column<Items>&... columns)
		{
			return pvRemoveHash(columnList, mUniqueHashes, columns...);
		}

		template<typename... Items>
		bool RemoveMultiHash(const ColumnList* columnList, const Column<Items>&... columns)
		{
			return pvRemoveHash(columnList, mMultiHashes, columns...);
		}

		void RemoveUniqueHashes() noexcept
		{
			mUniqueHashes.Clear(true);
		}

		void RemoveMultiHashes() noexcept
		{
			mMultiHashes.Clear(true);
		}

		void AddIndexes(const DataIndexes& indexes)
		{
			MOMO_ASSERT(mUniqueHashes.IsEmpty());
			MOMO_ASSERT(mMultiHashes.IsEmpty());
			const MemManagerPtr& memManagerPtr = mUniqueHashes.GetMemManager();
			try
			{
				mUniqueHashes.Reserve(indexes.mUniqueHashes.GetCount());
				mMultiHashes.Reserve(indexes.mMultiHashes.GetCount());
				for (const UniqueHash& uniqueHash : indexes.mUniqueHashes)
				{
					mUniqueHashes.AddBackNogrowVar(
						HashTraits(uniqueHash.GetHashTraits(), MemManagerPtr(memManagerPtr)),
						Offsets(uniqueHash.GetSortedOffsets(), MemManagerPtr(memManagerPtr)));
				}
				for (const MultiHash& multiHash : indexes.mMultiHashes)
				{
					mMultiHashes.AddBackNogrowVar(
						HashTraits(multiHash.GetHashTraits(), MemManagerPtr(memManagerPtr)),
						Offsets(multiHash.GetSortedOffsets(), MemManagerPtr(memManagerPtr)));
				}
			}
			catch (...)
			{
				RemoveUniqueHashes();
				RemoveMultiHashes();
				throw;
			}
		}

		typename UniqueHash::RawBounds FindRaws(const UniqueHash& uniqueHash, Raw* raw,
			VersionKeeper version) const
		{
			return uniqueHash.Find(raw, version);	//?
		}

		template<typename Hash, typename... Items>
		typename Hash::RawBounds FindRaws(const Hash& hash, const OffsetItemTuple<Items...>& tuple,
			VersionKeeper version) const
		{
			HashTupleKey<Items...> hashTupleKey{ tuple, pvGetHashCode<0>(tuple) };
			return hash.Find(hashTupleKey, version);
		}

		void ClearRaws() noexcept
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
			try
			{
				for (UniqueHash& uniqueHash : mUniqueHashes)
					uniqueHash.Add(raw);
				for (MultiHash& multiHash : mMultiHashes)
					multiHash.Add(raw);
			}
			catch (...)
			{
				for (UniqueHash& uniqueHash : mUniqueHashes)
					uniqueHash.RejectAdd();
				for (MultiHash& multiHash : mMultiHashes)
					multiHash.RejectAdd();
				throw;
			}
			for (UniqueHash& uniqueHash : mUniqueHashes)
				uniqueHash.AcceptAdd();
			for (MultiHash& multiHash : mMultiHashes)
				multiHash.AcceptAdd();
		}

		void RemoveRaw(Raw* raw)
		{
			try
			{
				for (UniqueHash& uniqueHash : mUniqueHashes)
					uniqueHash.PrepareRemove(raw);
				for (MultiHash& multiHash : mMultiHashes)
					multiHash.PrepareRemove(raw);
			}
			catch (...)
			{
				for (UniqueHash& uniqueHash : mUniqueHashes)
					uniqueHash.RejectRemove();
				for (MultiHash& multiHash : mMultiHashes)
					multiHash.RejectRemove();
				throw;
			}
			for (UniqueHash& uniqueHash : mUniqueHashes)
				uniqueHash.AcceptRemove();
			for (MultiHash& multiHash : mMultiHashes)
				multiHash.AcceptRemove(raw);
		}

		void UpdateRaw(Raw* oldRaw, Raw* newRaw)
		{
			try
			{
				for (UniqueHash& uniqueHash : mUniqueHashes)
				{
					if (uniqueHash.Add(newRaw, oldRaw))
						uniqueHash.PrepareRemove(oldRaw);
				}
				for (MultiHash& multiHash : mMultiHashes)
				{
					multiHash.Add(newRaw);
					multiHash.PrepareRemove(oldRaw);
				}
			}
			catch (...)
			{
				for (UniqueHash& uniqueHash : mUniqueHashes)
				{
					uniqueHash.RejectAdd(newRaw);
					uniqueHash.RejectRemove();
				}
				for (MultiHash& multiHash : mMultiHashes)
				{
					multiHash.RejectAdd();
					multiHash.RejectRemove();
				}
				throw;
			}
			for (UniqueHash& uniqueHash : mUniqueHashes)
			{
				uniqueHash.AcceptAdd(newRaw);
				uniqueHash.AcceptRemove();
			}
			for (MultiHash& multiHash : mMultiHashes)
			{
				multiHash.AcceptAdd();
				multiHash.AcceptRemove(oldRaw);
			}
		}

		template<typename Item, typename Assigner>
		void UpdateRaw(Raw* raw, size_t offset, const Item& item, Assigner assigner)
		{
			if (DataTraits::IsEqual(item, ColumnList::template GetByOffset<const Item>(raw, offset)))
				return assigner();
			HashMixedKey<Item> hashMixedKey{ raw, offset, std::addressof(item) };
			try
			{
				for (UniqueHash& uniqueHash : mUniqueHashes)
				{
					if (!HasOffset(uniqueHash, offset))
						continue;
					uniqueHash.Add(hashMixedKey);
					uniqueHash.PrepareRemove(raw);
				}
				for (MultiHash& multiHash : mMultiHashes)
				{
					if (!HasOffset(multiHash, offset))
						continue;
					multiHash.Add(hashMixedKey);
					multiHash.PrepareRemove(raw);
				}
				assigner();
			}
			catch (...)
			{
				for (UniqueHash& uniqueHash : mUniqueHashes)
				{
					uniqueHash.RejectAdd();
					uniqueHash.RejectRemove();
				}
				for (MultiHash& multiHash : mMultiHashes)
				{
					multiHash.RejectAdd();
					multiHash.RejectRemove();
				}
				throw;
			}
			for (UniqueHash& uniqueHash : mUniqueHashes)
			{
				uniqueHash.AcceptAdd();
				uniqueHash.AcceptRemove();
			}
			for (MultiHash& multiHash : mMultiHashes)
			{
				multiHash.AcceptAdd();
				multiHash.AcceptRemove(raw);
			}
		}

		template<typename RawFilter>
		void FilterRaws(RawFilter rawFilter) noexcept
		{
			for (UniqueHash& uniqueHash : mUniqueHashes)
				uniqueHash.FilterRaws(rawFilter);
			for (MultiHash& multiHash : mMultiHashes)
				multiHash.FilterRaws(rawFilter);
		}

		template<size_t columnCount>
		const UniqueHash* GetHash(const std::array<size_t, columnCount>& sortedOffsets,
			const UniqueHash*) const noexcept
		{
			return pvGetHash(mUniqueHashes, sortedOffsets);
		}

		template<size_t columnCount>
		const MultiHash* GetHash(const std::array<size_t, columnCount>& sortedOffsets,
			const MultiHash*) const noexcept
		{
			return pvGetHash(mMultiHashes, sortedOffsets);
		}

		template<size_t columnCount>
		const UniqueHash* GetFitUniqueHash(
			const std::array<size_t, columnCount>& sortedOffsets) const noexcept
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
		const MultiHash* GetFitMultiHash(
			const std::array<size_t, columnCount>& sortedOffsets) const noexcept
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
		static bool HasOffset(const Hash& hash, size_t offset) noexcept
		{
			const Offsets& sortedOffsets = hash.GetSortedOffsets();
			if (sortedOffsets.GetCount() < 16)
				return sortedOffsets.Contains(offset);
			return std::binary_search(sortedOffsets.GetBegin(), sortedOffsets.GetEnd(), offset);
		}

	private:
		template<typename Hash, typename... Items>
		const Hash* pvGetHash(const ColumnList* columnList, const Indexes<Hash>& hashes,
			const Column<Items>&... columns) const
		{
			static const size_t columnCount = sizeof...(columns);
			std::array<size_t, columnCount> offsets = {{ columnList->GetOffset(columns)... }};
			std::array<size_t, columnCount> sortedOffsets = GetSortedOffsets(offsets);
			return pvGetHash(hashes, sortedOffsets);
		}

		template<typename Hash, size_t columnCount>
		const Hash* pvGetHash(const Indexes<Hash>& hashes,
			const std::array<size_t, columnCount>& sortedOffsets) const
		{
			for (const Hash& hash : hashes)
			{
				const Offsets& curSortedOffsets = hash.GetSortedOffsets();
				bool equal = curSortedOffsets.GetCount() == columnCount
					&& std::equal(sortedOffsets.begin(), sortedOffsets.end(), curSortedOffsets.GetBegin());
				if (equal)
					return &hash;
			}
			return nullptr;
		}

		template<typename Hash, typename Raws, typename... Items>
		bool pvAddHash(const ColumnList* columnList, Indexes<Hash>& hashes, const Raws& raws,
			const Column<Items>&... columns)
		{
			static const size_t columnCount = sizeof...(columns);
			MOMO_STATIC_ASSERT(columnCount > 0);
			std::array<size_t, columnCount> offsets = {{ columnList->GetOffset(columns)... }};
			for (size_t offset : offsets)
			{
				(void)offset;
				MOMO_CHECK(!columnList->IsMutable(offset));
			}
			std::array<size_t, columnCount> sortedOffsets = GetSortedOffsets(offsets);
			if (pvGetHash(hashes, sortedOffsets) != nullptr)
				return false;
			auto hashFunc = [] (Raw* key, const size_t* offsets)
				{ return pvGetHashCode<void, Items...>(key, offsets); };
			auto equalFunc = [] (Raw* key1, Raw* key2, const size_t* offsets)
				{ return pvIsEqual<void, Items...>(key1, key2, offsets); };
			auto hashMixedFunc = [] (HashMixedKey<> key, const size_t* offsets)
				{ return pvGetHashCode<void, Items...>(key, offsets); };
			auto equalMixedFunc = [] (HashMixedKey<> key1, Raw* key2, const size_t* offsets)
				{ return pvIsEqual<void, Items...>(key1, key2, offsets); };
			const MemManagerPtr& memManagerPtr = hashes.GetMemManager();
			HashTraits hashTraits(hashFunc, equalFunc, hashMixedFunc, equalMixedFunc,
				Offsets(offsets.begin(), offsets.end(), MemManagerPtr(memManagerPtr)));
			Hash hash(std::move(hashTraits),
				Offsets(sortedOffsets.begin(), sortedOffsets.end(), MemManagerPtr(memManagerPtr)));
			for (Raw* raw : raws)
			{
				hash.Add(raw);
				hash.AcceptAdd();
			}
			hashes.Reserve(hashes.GetCount() + 1);
			hashes.AddBackNogrow(std::move(hash));
			return true;
		}

		template<typename Hash, typename... Items>
		bool pvRemoveHash(const ColumnList* columnList, Indexes<Hash>& hashes,
			const Column<Items>&... columns)
		{
			const Hash* hash = pvGetHash(columnList, hashes, columns...);
			if (hash == nullptr)
				return false;
			hashes.Remove(hash - hashes.GetItems());
			return true;
		}

		template<typename Void, typename Item, typename... Items>
		static size_t pvGetHashCode(Raw* key, const size_t* offsets)
		{
			size_t offset = *offsets;
			const Item& item = ColumnList::template GetByOffset<const Item>(key, offset);
			return DataTraits::GetHashCode(item, offset)
				+ pvGetHashCode<void, Items...>(key, offsets + 1);
		}

		template<typename Void>
		static size_t pvGetHashCode(Raw* /*key*/, const size_t* /*offsets*/) noexcept
		{
			return 0;
		}

		template<typename Void, typename Item, typename... Items>
		static size_t pvGetHashCode(HashMixedKey<> key, const size_t* offsets)
		{
			size_t offset = *offsets;
			const Item& item = (offset != key.offset)
				? ColumnList::template GetByOffset<const Item>(key.raw, offset)
				: *static_cast<const Item*>(key.item);
			return DataTraits::GetHashCode(item, offset)
				+ pvGetHashCode<void, Items...>(key, offsets + 1);
		}

		template<typename Void>
		static size_t pvGetHashCode(HashMixedKey<> /*key*/, const size_t* /*offsets*/) noexcept
		{
			return 0;
		}

		template<size_t index, typename... Items>
		static EnableIf<(index < sizeof...(Items)), size_t> pvGetHashCode(
			const OffsetItemTuple<Items...>& tuple)
		{
			const auto& pair = std::get<index>(tuple);
			const auto& item = pair.second;
			return DataTraits::GetHashCode(item, pair.first) + pvGetHashCode<index + 1>(tuple);
		}

		template<size_t index, typename... Items>
		static EnableIf<(index == sizeof...(Items)), size_t> pvGetHashCode(
			const OffsetItemTuple<Items...>& /*tuple*/) noexcept
		{
			return 0;
		}

		template<typename Void, typename Item, typename... Items>
		static bool pvIsEqual(Raw* key1, Raw* key2, const size_t* offsets)
		{
			size_t offset = *offsets;
			const Item& item1 = ColumnList::template GetByOffset<const Item>(key1, offset);
			const Item& item2 = ColumnList::template GetByOffset<const Item>(key2, offset);
			return DataTraits::IsEqual(item1, item2)
				&& pvIsEqual<void, Items...>(key1, key2, offsets + 1);
		}

		template<typename Void>
		static bool pvIsEqual(Raw* /*key1*/, Raw* /*key2*/, const size_t* /*offsets*/) noexcept
		{
			return true;
		}

		template<typename Void, typename Item, typename... Items>
		static bool pvIsEqual(HashMixedKey<> key1, Raw* key2, const size_t* offsets)
		{
			size_t offset = *offsets;
			const Item& item1 = (offset != key1.offset)
				? ColumnList::template GetByOffset<const Item>(key1.raw, offset)
				: *static_cast<const Item*>(key1.item);
			const Item& item2 = ColumnList::template GetByOffset<const Item>(key2, *offsets);
			return DataTraits::IsEqual(item1, item2)
				&& pvIsEqual<void, Items...>(key1, key2, offsets + 1);
		}

		template<typename Void>
		static bool pvIsEqual(HashMixedKey<> /*key1*/, Raw* /*key2*/,
			const size_t* /*offsets*/) noexcept
		{
			return true;
		}

	private:
		UniqueHashes mUniqueHashes;
		MultiHashes mMultiHashes;
	};
}

} // namespace momo

namespace std
{
	template<typename CL>
	struct iterator_traits<momo::internal::DataRawUniqueHashIterator<CL>>
		: public iterator_traits<typename CL::Raw* const*>
	{
	};

	template<typename CL, typename KI>
	struct iterator_traits<momo::internal::DataRawMultiHashIterator<CL, KI>>
		: public iterator_traits<typename CL::Raw* const*>
	{
	};
} // namespace std

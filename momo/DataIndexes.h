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

namespace experimental
{

namespace internal
{
	class DataHashSetSettings : public momo::HashSetSettings
	{
	public:
		static const CheckMode checkMode = CheckMode::assertion;
		static const ExtraCheckMode extraCheckMode = ExtraCheckMode::nothing;
		static const bool checkVersion = false;
	};

	class DataHashMultiMapSettings : public momo::HashMultiMapSettings
	{
	public:
		static const CheckMode checkMode = CheckMode::assertion;
		static const ExtraCheckMode extraCheckMode = ExtraCheckMode::nothing;
		static const bool checkKeyVersion = false;
		static const bool checkValueVersion = false;
	};

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
	class DataRawMultiHashIterator
		: private momo::internal::VersionKeeper<typename TColumnList::Settings>
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

		typedef momo::internal::VersionKeeper<Settings> VersionKeeper;

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
		typedef momo::internal::VersionKeeper<Settings> VersionKeeper;

		typedef momo::internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef momo::internal::NestedArrayIntCap<4, size_t, MemManagerPtr> Offsets;

		typedef std::function<size_t(Raw*, const size_t*)> HashFunc;
		typedef std::function<bool(Raw*, Raw*, const size_t*)> EqualFunc;

		template<typename... Items>
		struct HashTupleKey
		{
			OffsetItemTuple<Items...> tuple;
			size_t hashCode;
		};

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

			static const bool isFastNothrowHashable = false;

		public:
			explicit HashTraits(Offsets&& offsets, HashFunc&& hashFunc, EqualFunc&& equalFunc) noexcept
				: mOffsets(std::move(offsets)),
				mHashFunc(std::move(hashFunc)),
				mEqualFunc(std::move(equalFunc))
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

			bool IsEqual(Raw* key1, Raw* key2) const
			{
				return mEqualFunc(key1, key2, mOffsets.GetItems());
			}

			template<typename... Items>
			bool IsEqual(const HashTupleKey<Items...>& key1, Raw* key2) const
			{
				return pvIsEqual<0>(key1, key2);
			}

		private:
			template<size_t index, typename... Items>
			momo::internal::EnableIf<(index < sizeof...(Items)), bool> pvIsEqual(
				const HashTupleKey<Items...>& key1, Raw* key2) const
			{
				const auto& pair = std::get<index>(key1.tuple);
				const auto& item1 = pair.second;
				typedef typename std::decay<decltype(item1)>::type Item;
				const Item& item2 = ColumnList::template GetByOffset<const Item>(key2, pair.first);
				return DataTraits::IsEqual(item1, item2) && pvIsEqual<index + 1>(key1, key2);
			}

			template<size_t index, typename... Items>
			momo::internal::EnableIf<(index == sizeof...(Items)), bool> pvIsEqual(
				const HashTupleKey<Items...>& /*key1*/, Raw* /*key2*/) const noexcept
			{
				return true;
			}

		private:
			Offsets mOffsets;
			HashFunc mHashFunc;
			EqualFunc mEqualFunc;
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
				HashSetItemTraits<Raw*, Raw*, MemManagerPtr>, DataHashSetSettings> HashSet;

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
			explicit UniqueHash(Offsets&& offsets, Offsets&& sortedOffsets,
				HashFunc&& hashFunc, EqualFunc&& equalFunc)
				: mSortedOffsets(std::move(sortedOffsets)),
				mHashSet(HashTraits(std::move(offsets), std::move(hashFunc), std::move(equalFunc)),
					MemManagerPtr(mSortedOffsets.GetMemManager()))
			{
			}

			UniqueHash(UniqueHash&& uniqueHash) noexcept
				: mSortedOffsets(std::move(uniqueHash.mSortedOffsets)),
				mHashSet(std::move(uniqueHash.mHashSet)),
				mIterator(uniqueHash.mIterator)
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
				mIterator = uniqueHash.mIterator;
				return *this;
			}

			UniqueHash& operator=(const UniqueHash&) = delete;

			const Offsets& GetSortedOffsets() const noexcept
			{
				return mSortedOffsets;
			}

			void Reserve(size_t capacity)
			{
				mHashSet.Reserve(capacity);
			}

			void Clear() noexcept
			{
				mHashSet.Clear();
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

			void FindExisting(Raw* raw) noexcept
			{
				MOMO_ASSERT(!mIterator);
				try
				{
					mIterator = mHashSet.Find(raw);
					MOMO_ASSERT(!!mIterator);
				}
				catch (...)
				{
					mIterator = mHashSet.GetBegin();
					while (*mIterator != raw)
						++mIterator;
				}
			}

			void Add(Raw* raw)
			{
				MOMO_ASSERT(!mIterator);
				auto insRes = mHashSet.Insert(raw);
				if (!insRes.inserted)
					throw UniqueIndexViolation(*insRes.iterator, *this);
				mIterator = insRes.iterator;
			}

			void Insert(Raw* raw)
			{
				MOMO_ASSERT(!mIterator);
				mIterator = mHashSet.Insert(raw).iterator;
			}

			void Remove() noexcept
			{
				MOMO_ASSERT(!!mIterator);
				mHashSet.Remove(mIterator);
				mIterator = Iterator();
			}

			void ResetRaw(Raw* raw) noexcept
			{
				MOMO_ASSERT(!!mIterator);
				mHashSet.ResetKey(mIterator, raw);
				mIterator = Iterator();
			}

			void Accept() noexcept
			{
				MOMO_ASSERT(!!mIterator);
				mIterator = Iterator();
			}

			Raw* GetCurrentRaw() const noexcept
			{
				MOMO_ASSERT(!!mIterator);
				return *mIterator;
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
			Iterator mIterator;
		};

		class MultiHash
		{
		private:
			typedef momo::HashMultiMap<Raw*, Raw*, HashTraits, MemManagerPtr,
				HashMultiMapKeyValueTraits<Raw*, Raw*, MemManagerPtr>,
				DataHashMultiMapSettings> HashMultiMap;

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
			explicit MultiHash(Offsets&& offsets, Offsets&& sortedOffsets,
				HashFunc&& hashFunc, EqualFunc&& equalFunc)
				: mSortedOffsets(std::move(sortedOffsets)),
				mHashMultiMap(HashTraits(std::move(offsets), std::move(hashFunc), std::move(equalFunc)),
					MemManagerPtr(mSortedOffsets.GetMemManager())),
				mRawIndex(0)
			{
			}

			MultiHash(MultiHash&& multiHash) noexcept
				: mSortedOffsets(std::move(multiHash.mSortedOffsets)),
				mHashMultiMap(std::move(multiHash.mHashMultiMap)),
				mKeyIterator(multiHash.mKeyIterator),
				mRawIndex(multiHash.mRawIndex)
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
				mKeyIterator = multiHash.mKeyIterator;
				mRawIndex = multiHash.mRawIndex;
				return *this;
			}

			MultiHash& operator=(const MultiHash&) = delete;

			const Offsets& GetSortedOffsets() const noexcept
			{
				return mSortedOffsets;
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

			void FindExisting(Raw* raw) noexcept
			{
				MOMO_ASSERT(!mKeyIterator);
				try
				{
					mKeyIterator = mHashMultiMap.Find(raw);
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

			void Add(Raw* raw)
			{
				MOMO_ASSERT(!mKeyIterator);
				mKeyIterator = mHashMultiMap.InsertKey(raw);
				if (mKeyIterator->key != raw)
					mHashMultiMap.Add(mKeyIterator, raw);
				mRawIndex = mKeyIterator->values.GetCount();
			}

			void Remove() noexcept
			{
				MOMO_ASSERT(!!mKeyIterator);
				if (mRawIndex > 0)
				{
					mHashMultiMap.Remove(mKeyIterator, mRawIndex - 1);
				}
				else if (mKeyIterator->values.GetCount() > 0)
				{
					size_t valueCount = mKeyIterator->values.GetCount();
					mHashMultiMap.ResetKey(mKeyIterator, mKeyIterator->values[valueCount - 1]);
					mHashMultiMap.Remove(mKeyIterator, valueCount - 1);
				}
				else
				{
					mHashMultiMap.RemoveKey(mKeyIterator);
				}
				mKeyIterator = KeyIterator();
			}

			void Accept() noexcept
			{
				MOMO_ASSERT(!!mKeyIterator);
				mKeyIterator = KeyIterator();
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
			bool pvFindExisting(Raw* raw) noexcept
			{
				if (mKeyIterator->key == raw)
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
		template<typename Index>
		using Indexes = Array<Index, MemManagerPtr, ArrayItemTraits<Index, MemManagerPtr>,
			momo::internal::NestedArraySettings<ArraySettings<0, false>>>;	//?

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
			mUniqueHashes.Clear();
		}

		void RemoveMultiHashes() noexcept
		{
			mMultiHashes.Clear();
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
			size_t uniqueHashCount = mUniqueHashes.GetCount();
			size_t uniqueHashIndex = 0;
			size_t multiHashCount = mMultiHashes.GetCount();
			size_t multiHashIndex = 0;
			try
			{
				for (; uniqueHashIndex < uniqueHashCount; ++uniqueHashIndex)
					mUniqueHashes[uniqueHashIndex].Add(raw);
				for (; multiHashIndex < multiHashCount; ++multiHashIndex)
					mMultiHashes[multiHashIndex].Add(raw);
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

		void RemoveRaw(Raw* raw) noexcept
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
					mMultiHashes[multiHashIndex].Add(newRaw);
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
				MOMO_CHECK(!columnList->IsMutable(offset));
			std::array<size_t, columnCount> sortedOffsets = GetSortedOffsets(offsets);
			if (pvGetHash(hashes, sortedOffsets) != nullptr)
				return false;
			auto hashFunc = [] (Raw* raw, const size_t* offsets)
				{ return pvGetHashCode<void, Items...>(raw, offsets); };
			auto equalFunc = [] (Raw* raw1, Raw* raw2, const size_t* offsets)
				{ return pvIsEqual<void, Items...>(raw1, raw2, offsets); };
			const MemManagerPtr& memManagerPtr = hashes.GetMemManager();
			Hash newHash(Offsets(offsets.begin(), offsets.end(), MemManagerPtr(memManagerPtr)),
				Offsets(sortedOffsets.begin(), sortedOffsets.end(), MemManagerPtr(memManagerPtr)),
				hashFunc, equalFunc);
			for (Raw* raw : raws)
			{
				newHash.Add(raw);
				newHash.Accept();
			}
			hashes.Reserve(hashes.GetCount() + 1);
			hashes.AddBackNogrow(std::move(newHash));
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
		static size_t pvGetHashCode(Raw* raw, const size_t* offsets)
		{
			return pvGetHashCode<Item>(raw, *offsets)
				+ pvGetHashCode<void, Items...>(raw, offsets + 1);
		}

		template<typename Void>
		static size_t pvGetHashCode(Raw* /*raw*/, const size_t* /*offsets*/) noexcept
		{
			return 0;
		}

		template<size_t index, typename... Items>
		static momo::internal::EnableIf<(index < sizeof...(Items)), size_t> pvGetHashCode(
			const OffsetItemTuple<Items...>& tuple)
		{
			const auto& pair = std::get<index>(tuple);
			const auto& item = pair.second;
			return pvGetHashCode(item, pair.first) + pvGetHashCode<index + 1>(tuple);	//?
		}

		template<size_t index, typename... Items>
		static momo::internal::EnableIf<(index == sizeof...(Items)), size_t> pvGetHashCode(
			const OffsetItemTuple<Items...>& /*tuple*/) noexcept
		{
			return 0;
		}

		template<typename Item>
		static size_t pvGetHashCode(Raw* raw, size_t offset)
		{
			const Item& item = ColumnList::template GetByOffset<const Item>(raw, offset);
			return pvGetHashCode(item, offset);
		}

		template<typename Item>
		static size_t pvGetHashCode(const Item& item, size_t /*offset*/)
		{
			return DataTraits::GetHashCode(item);	//?
		}

		template<typename Void, typename Item, typename... Items>
		static bool pvIsEqual(Raw* raw1, Raw* raw2, const size_t* offsets)
		{
			const Item& item1 = ColumnList::template GetByOffset<const Item>(raw1, *offsets);
			const Item& item2 = ColumnList::template GetByOffset<const Item>(raw2, *offsets);
			return DataTraits::IsEqual(item1, item2)
				&& pvIsEqual<void, Items...>(raw1, raw2, offsets + 1);
		}

		template<typename Void>
		static bool pvIsEqual(Raw* /*raw1*/, Raw* /*raw2*/, const size_t* /*offsets*/) noexcept
		{
			return true;
		}

	private:
		UniqueHashes mUniqueHashes;
		MultiHashes mMultiHashes;
	};
}

} // namespace experimental

} // namespace momo

namespace std
{
	template<typename CL>
	struct iterator_traits<momo::experimental::internal::DataRawUniqueHashIterator<CL>>
		: public iterator_traits<typename CL::Raw* const*>
	{
	};

	template<typename CL, typename KI>
	struct iterator_traits<momo::experimental::internal::DataRawMultiHashIterator<CL, KI>>
		: public iterator_traits<typename CL::Raw* const*>
	{
	};
} // namespace std

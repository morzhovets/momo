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

	template<typename TColumnList>
	class DataRawHashIterator
	{
	protected:
		typedef TColumnList ColumnList;

	public:
		typedef typename ColumnList::Settings Settings;
		typedef typename ColumnList::Raw Raw;

		typedef Raw* const& Reference;
		typedef Raw* const* Pointer;

		typedef DataRawHashIterator ConstIterator;

	public:
		explicit DataRawHashIterator() MOMO_NOEXCEPT
			: mRaw(nullptr),
			mRaws(nullptr),
			mRawIndex(0)
		{
		}

		explicit DataRawHashIterator(Raw* raw, Raw* const* raws, size_t rawIndex) MOMO_NOEXCEPT
			: mRaw(raw),
			mRaws(raws),
			mRawIndex(rawIndex)
		{
			MOMO_ASSERT(pvCheckParams(raw, raws, rawIndex));
		}

		DataRawHashIterator& operator+=(ptrdiff_t diff)
		{
			ptrdiff_t newRawIndex = (ptrdiff_t)mRawIndex + diff;
			MOMO_CHECK(newRawIndex >= 0);
			MOMO_CHECK(pvCheckParams(mRaw, mRaws, (size_t)newRawIndex));
			mRawIndex = (size_t)newRawIndex;
			return *this;
		}

		ptrdiff_t operator-(ConstIterator iter) const
		{
			MOMO_CHECK(mRaw == iter.mRaw && mRaws == iter.mRaws);
			return mRawIndex - iter.mRawIndex;
		}

		Pointer operator->() const
		{
			MOMO_CHECK(mRaw != nullptr);
			MOMO_CHECK(mRaws != nullptr || mRawIndex == 0);
			return (mRawIndex > 0) ? &mRaws[mRawIndex - 1] : &mRaw;
		}

		bool operator==(ConstIterator iter) const MOMO_NOEXCEPT
		{
			return mRaw == iter.mRaw && mRaws == iter.mRaws && mRawIndex == iter.mRawIndex;
		}

		bool operator<(ConstIterator iter) const
		{
			MOMO_CHECK(mRaw == iter.mRaw && mRaws == iter.mRaws);
			return mRawIndex < iter.mRawIndex;
		}

		MOMO_MORE_ARRAY_ITERATOR_OPERATORS(DataRawHashIterator)

	private:
		static bool pvCheckParams(Raw* raw, Raw* const* raws, size_t rawIndex) MOMO_NOEXCEPT
		{
			if (raw == nullptr)
				return raws == nullptr && rawIndex == 0;
			if (raws == nullptr)
				return rawIndex <= 1;
			return true;
		}

	private:
		Raw* mRaw;
		Raw* const* mRaws;
		size_t mRawIndex;
	};

	template<typename TColumnList>
	class DataRawHashBounds
	{
	protected:
		typedef TColumnList ColumnList;

	public:
		typedef typename ColumnList::Settings Settings;
		typedef typename ColumnList::Raw Raw;

		typedef DataRawHashIterator<ColumnList> Iterator;

		typedef DataRawHashBounds ConstBounds;

	public:
		explicit DataRawHashBounds() MOMO_NOEXCEPT
			: mRaw(nullptr),
			mRaws(nullptr),
			mRawCount(0)
		{
		}

		explicit DataRawHashBounds(Raw* raw, Raw* const* raws, size_t rawCount) MOMO_NOEXCEPT
			: mRaw(raw),
			mRaws(raws),
			mRawCount(rawCount)
		{
			//MOMO_ASSERT(pvCheckParams(raw, raws, rawCount));
		}

		Iterator GetBegin() const MOMO_NOEXCEPT
		{
			return Iterator(mRaw, mRaws, 0);
		}

		Iterator GetEnd() const MOMO_NOEXCEPT
		{
			return Iterator(mRaw, mRaws, mRawCount);
		}

		MOMO_FRIENDS_BEGIN_END(const DataRawHashBounds&, Iterator)

		size_t GetCount() const MOMO_NOEXCEPT
		{
			return mRawCount;
		}

	private:
		Raw* mRaw;
		Raw* const* mRaws;
		size_t mRawCount;
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
		typedef momo::internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef momo::internal::NestedArrayIntCap<4, size_t, MemManagerPtr> Offsets;

		typedef std::function<size_t(Raw*)> HashFunc;
		typedef std::function<bool(Raw*, Raw*)> EqualFunc;

		template<typename... Items>
		struct HashTupleKey
		{
			OffsetItemTuple<Items...> tuple;
			size_t hashCode;
			const ColumnList* columnList;
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
			explicit HashTraits(HashFunc&& hashFunc, EqualFunc&& equalFunc)
				: mHashFunc(std::move(hashFunc)),
				mEqualFunc(std::move(equalFunc))
			{
			}

			size_t GetHashCode(Raw* key) const
			{
				return mHashFunc(key);
			}

			template<typename... Items>
			size_t GetHashCode(const HashTupleKey<Items...>& key) const MOMO_NOEXCEPT
			{
				return key.hashCode;
			}

			bool IsEqual(Raw* key1, Raw* key2) const
			{
				return mEqualFunc(key1, key2);
			}

			template<typename... Items>
			bool IsEqual(const HashTupleKey<Items...>& key1, Raw* key2) const
			{
				return pvIsEqual<0>(key1, key2);
			}

		private:
			template<size_t index, typename... Items,
				typename std::enable_if<(index < sizeof...(Items)), int>::type = 0>
			bool pvIsEqual(const HashTupleKey<Items...>& key1, Raw* key2) const
			{
				const auto& pair = std::get<index>(key1.tuple);
				const auto& item1 = pair.second;
				typedef typename std::decay<decltype(item1)>::type Item;
				const Item& item2 = key1.columnList->template GetByOffset<const Item>(key2, pair.first);
				return DataTraits::IsEqual(item1, item2) && pvIsEqual<index + 1>(key1, key2);
			}

			template<size_t index, typename... Items,
				typename std::enable_if<(index == sizeof...(Items)), int>::type = 0>
			bool pvIsEqual(const HashTupleKey<Items...>& /*key1*/, Raw* /*key2*/) const MOMO_NOEXCEPT
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
			typedef DataRawHashBounds<ColumnList> RawBounds;

		public:
			explicit UniqueHash(Offsets&& sortedOffsets, HashFunc&& hashFunc, EqualFunc&& equalFunc)
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
				return pvFind(raw);
			}

			template<typename... Items>
			RawBounds Find(const HashTupleKey<Items...>& hashTupleKey) const
			{
				return pvFind(hashTupleKey);
			}

			void FindExisting(Raw* raw) MOMO_NOEXCEPT
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

			void Remove() MOMO_NOEXCEPT
			{
				MOMO_ASSERT(!!mIterator);
				mHashSet.Remove(mIterator);
				mIterator = Iterator();
			}

			void ResetRaw(Raw* raw) MOMO_NOEXCEPT
			{
				MOMO_ASSERT(!!mIterator);
				mHashSet.ResetKey(mIterator, raw);
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
				return *mIterator;
			}

			template<typename RawFilter>
			void FilterRaws(RawFilter rawFilter) MOMO_NOEXCEPT
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
				return !!iter ? RawBounds(*iter, nullptr, 1) : RawBounds();
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
			typedef DataRawHashBounds<ColumnList> RawBounds;

		public:
			explicit MultiHash(Offsets&& sortedOffsets, HashFunc&& hashFunc, EqualFunc&& equalFunc)
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

			template<typename... Items>
			RawBounds Find(const HashTupleKey<Items...>& hashTupleKey) const
			{
				KeyIterator keyIter = mHashMultiMap.Find(hashTupleKey);
				if (!keyIter)
					return RawBounds();
				return RawBounds(keyIter->key, keyIter->values.GetBegin(),
					keyIter->values.GetCount() + 1);
			}

			void FindExisting(Raw* raw) MOMO_NOEXCEPT
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
					mHashMultiMap.ResetKey(mKeyIterator, mKeyIterator->values[valueCount - 1]);
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
			bool pvFindExisting(Raw* raw) MOMO_NOEXCEPT
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
		template<typename Hash>
		using Hashes = Array<Hash, MemManagerPtr, ArrayItemTraits<Hash, MemManagerPtr>,
			momo::internal::NestedArraySettings<ArraySettings<0, false>>>;	//?

		typedef Hashes<UniqueHash> UniqueHashes;
		typedef Hashes<MultiHash> MultiHashes;

	public:
		explicit DataIndexes(MemManager& memManager) MOMO_NOEXCEPT
			: mUniqueHashes(MemManagerPtr(memManager)),
			mMultiHashes(MemManagerPtr(memManager))
		{
		}

		DataIndexes(DataIndexes&& indexes) MOMO_NOEXCEPT
			: mUniqueHashes(std::move(indexes.mUniqueHashes)),
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
		const UniqueHash* AddUniqueHash(const ColumnList* columnList, const Raws& raws,
			const Column<Items>&... columns)
		{
			return pvAddHash(columnList, mUniqueHashes, raws, columns...);
		}

		template<typename Raws, typename... Items>
		const MultiHash* AddMultiHash(const ColumnList* columnList, const Raws& raws,
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

		typename UniqueHash::RawBounds FindRaws(const UniqueHash& uniqueHash, Raw* raw) const
		{
			return uniqueHash.Find(raw);	//?
		}

		template<typename Hash, typename... Items>
		typename Hash::RawBounds FindRaws(const ColumnList* columnList, const Hash& hash,
			const OffsetItemTuple<Items...>& tuple) const
		{
			HashTupleKey<Items...> hashTupleKey{ tuple, pvGetHashCode<0>(tuple), columnList };
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
		const UniqueHash* GetFitUniqueHash(
			const std::array<size_t, columnCount>& sortedOffsets) const MOMO_NOEXCEPT
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
			const std::array<size_t, columnCount>& sortedOffsets) const MOMO_NOEXCEPT
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
		template<typename Hashes, typename... Items>
		const typename Hashes::Item* pvGetHash(const ColumnList* columnList, const Hashes& hashes,
			const Column<Items>&... columns) const
		{
			static const size_t columnCount = sizeof...(columns);
			std::array<size_t, columnCount> offsets = {{ columnList->GetOffset(columns)... }};	// C++11
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

		template<typename Hashes, typename Raws, typename... Items>
		const typename Hashes::Item* pvAddHash(const ColumnList* columnList, Hashes& hashes,
			const Raws& raws, const Column<Items>&... columns)
		{
			static const size_t columnCount = sizeof...(columns);
			MOMO_STATIC_ASSERT(columnCount > 0);
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
			auto hashFunc = [columnList, offsets] (Raw* raw)
				{ return pvGetHashCode<void, Items...>(columnList, raw, offsets.data()); };
			auto equalFunc = [columnList, offsets] (Raw* raw1, Raw* raw2)
				{ return pvIsEqual<void, Items...>(columnList, raw1, raw2, offsets.data()); };
			Offsets newHashOffsets(sortedOffsets.begin(), sortedOffsets.end(),
				MemManagerPtr(hashes.GetMemManager()));
			typename Hashes::Item newHash(std::move(newHashOffsets), hashFunc, equalFunc);
			for (Raw* raw : raws)
			{
				newHash.Add(raw);
				newHash.Accept();
			}
			hashes.Reserve(hashes.GetCount() + 1);
			hashes.AddBackNogrow(std::move(newHash));
			return hashes.GetItems() + hashes.GetCount() - 1;
		}

		template<typename Hashes, typename... Items>
		bool pvRemoveHash(const ColumnList* columnList, Hashes& hashes,
			const Column<Items>&... columns)
		{
			const auto* hash = pvGetHash(columnList, hashes, columns...);
			if (hash == nullptr)
				return false;
			hashes.Remove(hash - hashes.GetItems(), 1);
			return true;
		}

		template<typename Void, typename Item, typename... Items>
		static size_t pvGetHashCode(const ColumnList* columnList, Raw* raw, const size_t* offsets)
		{
			return pvGetHashCode<Item>(columnList, raw, *offsets)
				+ pvGetHashCode<void, Items...>(columnList, raw, offsets + 1);
		}

		template<typename Void>
		static size_t pvGetHashCode(const ColumnList* /*columnList*/, Raw* /*raw*/,
			const size_t* /*offsets*/) MOMO_NOEXCEPT
		{
			return 0;
		}

		template<size_t index, typename... Items,
			typename std::enable_if<(index < sizeof...(Items)), int>::type = 0>
		static size_t pvGetHashCode(const OffsetItemTuple<Items...>& tuple)
		{
			const auto& pair = std::get<index>(tuple);
			const auto& item = pair.second;
			return pvGetHashCode(item, pair.first) + pvGetHashCode<index + 1>(tuple);	//?
		}

		template<size_t index, typename... Items,
			typename std::enable_if<(index == sizeof...(Items)), int>::type = 0>
		static size_t pvGetHashCode(const OffsetItemTuple<Items...>& /*tuple*/) MOMO_NOEXCEPT
		{
			return 0;
		}

		template<typename Item>
		static size_t pvGetHashCode(const ColumnList* columnList, Raw* raw, size_t offset)
		{
			const Item& item = columnList->template GetByOffset<const Item>(raw, offset);
			return pvGetHashCode(item, offset);
		}

		template<typename Item>
		static size_t pvGetHashCode(const Item& item, size_t /*offset*/)
		{
			return DataTraits::GetHashCode(item);	//?
		}

		template<typename Void, typename Item, typename... Items>
		static bool pvIsEqual(const ColumnList* columnList, Raw* raw1, Raw* raw2,
			const size_t* offsets)
		{
			const Item& item1 = columnList->template GetByOffset<const Item>(raw1, *offsets);
			const Item& item2 = columnList->template GetByOffset<const Item>(raw2, *offsets);
			return DataTraits::IsEqual(item1, item2)
				&& pvIsEqual<void, Items...>(columnList, raw1, raw2, offsets + 1);
		}

		template<typename Void>
		static bool pvIsEqual(const ColumnList* /*columnList*/, Raw* /*raw1*/, Raw* /*raw2*/,
			const size_t* /*offsets*/) MOMO_NOEXCEPT
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
	struct iterator_traits<momo::experimental::internal::DataRawHashIterator<CL>>
		: public iterator_traits<typename CL::Raw* const*>
	{
	};
} // namespace std

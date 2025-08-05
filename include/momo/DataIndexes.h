/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/DataIndexes.h

  namespace momo:
    enum class DataUniqueHashIndex
    enum class DataMultiHashIndex

\**********************************************************/

#pragma once

#include "HashMultiMap.h"
#include "SegmentedArray.h"
#include "RadixSorter.h"

namespace momo
{

enum class DataUniqueHashIndex : ptrdiff_t
{
	empty = -1,
};

enum class DataMultiHashIndex : ptrdiff_t
{
	empty = -1,
};

namespace internal
{
	template<typename TRaw, typename TSettings>
	class DataRawUniqueHashIterator : public ArrayIteratorBase
	{
	public:
		typedef TRaw Raw;
		typedef TSettings Settings;

		typedef Raw* const& Reference;
		typedef Raw* const* Pointer;

		typedef DataRawUniqueHashIterator ConstIterator;

	public:
		explicit DataRawUniqueHashIterator() noexcept
			: mRaw(nullptr),
			mRawIndex(0)
		{
		}

		explicit DataRawUniqueHashIterator(Raw* raw, size_t rawIndex) noexcept
			: mRaw(raw),
			mRawIndex(static_cast<ptrdiff_t>(rawIndex))
		{
		}

		//operator ConstIterator() const noexcept

		DataRawUniqueHashIterator& operator+=(ptrdiff_t diff)
		{
			ptrdiff_t newRawIndex = mRawIndex + diff;
			MOMO_CHECK(0 <= newRawIndex && newRawIndex <= ((mRaw != nullptr) ? 1 : 0));
			mRawIndex = newRawIndex;
			return *this;
		}

		friend ptrdiff_t operator-(DataRawUniqueHashIterator iter1, DataRawUniqueHashIterator iter2)
		{
			MOMO_CHECK(iter1.mRaw == iter2.mRaw);
			return iter1.mRawIndex - iter2.mRawIndex;
		}

		Pointer operator->() const
		{
			MOMO_CHECK(mRaw != nullptr && mRawIndex == 0);
			return &mRaw;
		}

		friend bool operator==(DataRawUniqueHashIterator iter1,
			DataRawUniqueHashIterator iter2) noexcept
		{
			return iter1.mRaw == iter2.mRaw && iter1.mRawIndex == iter2.mRawIndex;
		}

		friend auto operator<=>(DataRawUniqueHashIterator iter1, DataRawUniqueHashIterator iter2)
		{
			MOMO_CHECK(iter1.mRaw == iter2.mRaw);
			return iter1.mRawIndex <=> iter2.mRawIndex;
		}

	private:
		Raw* mRaw;
		ptrdiff_t mRawIndex;
	};

	template<typename TRaw, typename TSettings>
	class DataRawUniqueHashBounds : public Rangeable
	{
	public:
		typedef TRaw Raw;
		typedef TSettings Settings;

		typedef DataRawUniqueHashIterator<Raw, Settings> Iterator;

		typedef DataRawUniqueHashBounds ConstBounds;

	public:
		explicit DataRawUniqueHashBounds(Raw* raw = nullptr) noexcept
			: mRaw(raw)
		{
		}

		//operator ConstBounds() const noexcept

		Iterator GetBegin() const noexcept
		{
			return Iterator(mRaw, 0);
		}

		Iterator GetEnd() const noexcept
		{
			return Iterator(mRaw, GetCount());
		}

		size_t GetCount() const noexcept
		{
			return (mRaw != nullptr) ? 1 : 0;
		}

	private:
		Raw* mRaw;
	};

	template<typename TRawIterator, typename TSettings>
	class MOMO_EMPTY_BASES DataRawMultiHashIterator
		: private VersionKeeper<TSettings>,
		public ArrayIteratorBase
	{
	public:
		typedef TRawIterator RawIterator;
		typedef TSettings Settings;

		typedef std::iter_reference_t<RawIterator> Reference;
		typedef typename std::iterator_traits<RawIterator>::pointer Pointer;

		typedef DataRawMultiHashIterator ConstIterator;

		typedef std::decay_t<Reference> RawPtr;

		typedef internal::VersionKeeper<Settings> VersionKeeper;

	public:
		explicit DataRawMultiHashIterator() noexcept
			: mRaw0(nullptr),
			mRawBegin(),
			mRawIndex(0)
		{
		}

		explicit DataRawMultiHashIterator(RawPtr raw0, RawIterator rawBegin, size_t rawIndex,
			VersionKeeper version) noexcept
			: VersionKeeper(version),
			mRaw0(raw0),
			mRawBegin(rawBegin),
			mRawIndex(static_cast<ptrdiff_t>(rawIndex))
		{
		}

		//operator ConstIterator() const noexcept

		DataRawMultiHashIterator& operator+=(ptrdiff_t diff)
		{
			if (diff != 0)
			{
				VersionKeeper::Check();
				MOMO_CHECK(mRaw0 != nullptr);
				ptrdiff_t newRawIndex = mRawIndex + diff;
				MOMO_CHECK(newRawIndex >= 0);
				MOMO_CHECK(mRawBegin != RawIterator() || newRawIndex <= 1);
				mRawIndex = newRawIndex;
			}
			return *this;
		}

		friend ptrdiff_t operator-(DataRawMultiHashIterator iter1, DataRawMultiHashIterator iter2)
		{
			MOMO_CHECK(iter1.mRaw0 == iter2.mRaw0);
			return iter1.mRawIndex - iter2.mRawIndex;
		}

		Pointer operator->() const
		{
			VersionKeeper::Check();
			if (mRawIndex > 0)
			{
				MOMO_CHECK(mRawBegin != RawIterator());
				return &mRawBegin[mRawIndex - 1];
			}
			else
			{
				MOMO_CHECK(mRaw0 != nullptr);
				return &mRaw0;
			}
		}

		friend bool operator==(DataRawMultiHashIterator iter1,
			DataRawMultiHashIterator iter2) noexcept
		{
			return iter1.mRaw0 == iter2.mRaw0 && iter1.mRawIndex == iter2.mRawIndex;
		}

		friend auto operator<=>(DataRawMultiHashIterator iter1, DataRawMultiHashIterator iter2)
		{
			MOMO_CHECK(iter1.mRaw0 == iter2.mRaw0);
			return iter1.mRawIndex <=> iter2.mRawIndex;
		}

	private:
		RawPtr mRaw0;
		RawIterator mRawBegin;
		ptrdiff_t mRawIndex;
	};

	template<typename TRawIterator, typename TSettings>
	class MOMO_EMPTY_BASES DataRawMultiHashBounds
		: private VersionKeeper<TSettings>,
		public Rangeable
	{
	public:
		typedef TRawIterator RawIterator;
		typedef TSettings Settings;

		typedef DataRawMultiHashIterator<RawIterator, Settings> Iterator;

		typedef DataRawMultiHashBounds ConstBounds;

		typedef typename Iterator::RawPtr RawPtr;

		typedef internal::VersionKeeper<Settings> VersionKeeper;

	public:
		explicit DataRawMultiHashBounds() noexcept
			: mRaw0(nullptr),
			mRawBegin(),
			mRawCount(0)
		{
		}

		explicit DataRawMultiHashBounds(RawPtr raw0, RawIterator rawBegin, size_t rawCount,
			VersionKeeper version) noexcept
			: VersionKeeper(version),
			mRaw0(raw0),
			mRawBegin(rawBegin),
			mRawCount(rawCount)
		{
		}

		//operator ConstBounds() const noexcept

		Iterator GetBegin() const noexcept
		{
			return Iterator(mRaw0, mRawBegin, 0, *this);
		}

		Iterator GetEnd() const noexcept
		{
			return Iterator(mRaw0, mRawBegin, mRawCount, *this);
		}

		size_t GetCount() const noexcept
		{
			return mRawCount;
		}

	private:
		RawPtr mRaw0;
		RawIterator mRawBegin;
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

		typedef DataUniqueHashIndex UniqueHashIndex;
		typedef DataMultiHashIndex MultiHashIndex;

		template<typename... Items>
		using OffsetItemTuple = std::tuple<std::pair<size_t, const Items&>...>;

	private:
		typedef internal::VersionKeeper<Settings> VersionKeeper;

		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef NestedArrayIntCap<4, size_t, MemManagerPtr> Offsets;

		template<typename Item = void>
		struct MixedRaw
		{
			Raw* raw;
			size_t offset;
			const Item* item;
		};

		typedef size_t (*RawHasher)(Raw*, const size_t*);
		typedef bool (*RawEqualComparer)(Raw*, Raw*, const size_t*);
		typedef size_t (*MixedRawHasher)(const MixedRaw<>&, const size_t*);
		typedef bool (*MixedRawEqualComparer)(const MixedRaw<>&, Raw*, const size_t*);

		class HashTraits : public momo::HashTraits<Raw*, typename DataTraits::HashBucket>
		{
		public:
			template<typename KeyArg>
			struct IsValidKeyArg : public std::false_type
			{
			};

			template<typename... Items>
			struct IsValidKeyArg<OffsetItemTuple<Items...>> : public std::true_type
			{
			};

			template<typename Item>
			struct IsValidKeyArg<MixedRaw<Item>> : public std::true_type
			{
			};

			static const bool isFastNothrowHashable = false;

		public:
			explicit HashTraits(RawHasher rawHasher, RawEqualComparer rawEqualComp,
				MixedRawHasher mixedRawHasher, MixedRawEqualComparer mixedRawEqualComp,
				Offsets&& offsets) noexcept
				: mRawHasher(rawHasher),
				mRawEqualComparer(rawEqualComp),
				mMixedRawHasher(mixedRawHasher),
				mMixedRawEqualComparer(mixedRawEqualComp),
				mOffsets(std::move(offsets))
			{
			}

			explicit HashTraits(const HashTraits& hashTraits, MemManagerPtr&& memManagerPtr)
				: mRawHasher(hashTraits.mRawHasher),
				mRawEqualComparer(hashTraits.mRawEqualComparer),
				mMixedRawHasher(hashTraits.mMixedRawHasher),
				mMixedRawEqualComparer(hashTraits.mMixedRawEqualComparer),
				mOffsets(hashTraits.mOffsets, std::move(memManagerPtr))
			{
			}

			size_t GetHashCode(Raw* key) const
			{
				return mRawHasher(key, mOffsets.GetItems());
			}

			template<typename... Items>
			size_t GetHashCode(const OffsetItemTuple<Items...>& key) const
			{
				size_t hashCode = 0;
				auto tupleHasher = [&hashCode] (const auto&... pairs)
					{ (DataTraits::AccumulateHashCode(hashCode, pairs.second, pairs.first), ...); };
				std::apply(tupleHasher, key);
				return hashCode;
			}

			template<typename Item>
			size_t GetHashCode(const MixedRaw<Item>& key) const
			{
				return mMixedRawHasher({ key.raw, key.offset, key.item }, mOffsets.GetItems());
			}

			bool IsEqual(Raw* key1, Raw* key2) const
			{
				return mRawEqualComparer(key1, key2, mOffsets.GetItems());
			}

			template<typename... Items>
			bool IsEqual(const OffsetItemTuple<Items...>& key1, Raw* key2) const
			{
				auto tupleEqualComp = [key2] (const auto&... pairs)
				{
					return (DataTraits::IsEqual(pairs.second,
						ColumnList::template GetByOffset<const Items>(key2, pairs.first)) && ...);
				};
				return std::apply(tupleEqualComp, key1);
			}

			template<typename Item>
			bool IsEqual(const MixedRaw<Item>& key1, Raw* key2) const
			{
				return mMixedRawEqualComparer({ key1.raw, key1.offset, key1.item }, key2,
					mOffsets.GetItems());
			}

		private:
			RawHasher mRawHasher;
			RawEqualComparer mRawEqualComparer;
			MixedRawHasher mMixedRawHasher;
			MixedRawEqualComparer mMixedRawEqualComparer;
			Offsets mOffsets;
		};

		class UniqueHash
		{
		private:
			typedef momo::HashSet<Raw*, HashTraits, MemManagerPtr,
				HashSetItemTraits<Raw*, MemManagerPtr>, NestedHashSetSettings> HashSet;

			typedef typename HashSet::ConstPosition Position;

		public:
			typedef DataRawUniqueHashBounds<Raw, Settings> RawBounds;

			typedef UniqueHashIndex Index;

		public:
			explicit UniqueHash(HashTraits&& hashTraits, Offsets&& sortedOffsets)
				: mSortedOffsets(std::move(sortedOffsets)),
				mHashSet(std::move(hashTraits), MemManagerPtr(mSortedOffsets.GetMemManager()))
			{
			}

			UniqueHash(UniqueHash&& uniqueHash) noexcept
				: mSortedOffsets(std::move(uniqueHash.mSortedOffsets)),
				mHashSet(std::move(uniqueHash.mHashSet)),
				mPositionAdd(uniqueHash.mPositionAdd),
				mPositionRemove(uniqueHash.mPositionRemove)
			{
			}

			UniqueHash(const UniqueHash&) = delete;

			~UniqueHash() noexcept = default;

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
			RawBounds Find(const OffsetItemTuple<Items...>& tuple, VersionKeeper /*version*/) const
			{
				return pvFind(tuple);
			}

			Raw* Add(Raw* raw, Raw* oldRaw = nullptr)
			{
				MOMO_ASSERT(!mPositionAdd);
				auto insRes = mHashSet.Insert(raw);
				if (insRes.inserted || *insRes.position == oldRaw)
					mPositionAdd = insRes.position;
				return *insRes.position;
			}

			template<typename Item>
			Raw* Add(const MixedRaw<Item>& mixedRaw)
			{
				MOMO_ASSERT(!mPositionAdd);
				Position pos = mHashSet.Find(mixedRaw);
				if (!!pos)
					return *pos;
				mPositionAdd = mHashSet.Add(pos, mixedRaw.raw);
				return mixedRaw.raw;
			}

			void RejectAdd() noexcept
			{
				if (!!mPositionAdd)
					mHashSet.Remove(mPositionAdd);
				mPositionAdd = Position();
			}

			void RejectAdd(Raw* raw) noexcept
			{
				if (!!mPositionAdd && *mPositionAdd == raw)
					mHashSet.Remove(mPositionAdd);
				mPositionAdd = Position();
			}

			void AcceptAdd() noexcept
			{
				mPositionAdd = Position();
			}

			void AcceptAdd(Raw* raw) noexcept
			{
				if (!!mPositionAdd)
					mHashSet.ResetKey(mPositionAdd, raw);
				mPositionAdd = Position();
			}

			void PrepareRemove(Raw* raw)
			{
				MOMO_ASSERT(!mPositionRemove);
				mPositionRemove = mHashSet.Find(raw);
				MOMO_ASSERT(!!mPositionRemove);
			}

			void RejectRemove() noexcept
			{
				mPositionRemove = Position();
			}

			void AcceptRemove() noexcept
			{
				if (!!mPositionRemove)
					mHashSet.Remove(mPositionRemove);
				mPositionRemove = Position();
			}

			template<internal::conceptPredicate<Raw*> RawFilter>
			void FilterRaws(FastCopyableFunctor<RawFilter> rawFilter) noexcept
			{
				mHashSet.Remove([rawFilter] (Raw* raw) noexcept { return !rawFilter(raw); });
			}

		private:
			template<typename Key>
			RawBounds pvFind(const Key& key) const
			{
				Position pos = mHashSet.Find(key);
				return RawBounds(!!pos ? *pos : nullptr);
			}

		private:
			Offsets mSortedOffsets;
			HashSet mHashSet;
			Position mPositionAdd;
			Position mPositionRemove;
		};

		class MultiHash
		{
		private:
			typedef momo::HashMultiMap<Raw*, Raw*, HashTraits, MemManagerPtr,
				HashMultiMapKeyValueTraits<Raw*, Raw*, MemManagerPtr>,
				NestedHashMultiMapSettings> HashMultiMap;

			typedef typename HashMultiMap::ConstKeyIterator ConstKeyIterator;
			typedef typename HashMultiMap::KeyIterator KeyIterator;

			static const size_t logInitialSegmentSize = 6;

			typedef momo::SegmentedArraySettings<momo::SegmentedArrayItemCountFunc::sqrt,
				logInitialSegmentSize> SegmentedArraySettings;

		public:
			typedef DataRawMultiHashBounds<typename ConstKeyIterator::Reference::Iterator,
				Settings> RawBounds;

			typedef MultiHashIndex Index;

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

			~MultiHash() noexcept = default;

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
			RawBounds Find(const OffsetItemTuple<Items...>& tuple, VersionKeeper version) const
			{
				ConstKeyIterator keyIter = mHashMultiMap.Find(tuple);
				return RawBounds(keyIter->key, keyIter->GetBegin(), keyIter->GetCount() + 1, version);
			}

			void Add(Raw* raw)
			{
				MOMO_ASSERT(!mKeyIteratorAdd);
				KeyIterator keyIter = mHashMultiMap.InsertKey(raw);
				if (keyIter->key != raw)
					pvAdd(keyIter, raw);
				mKeyIteratorAdd = keyIter;
			}

			template<typename Item>
			void Add(const MixedRaw<Item>& mixedRaw)
			{
				MOMO_ASSERT(!mKeyIteratorAdd);
				KeyIterator keyIter = mHashMultiMap.Find(mixedRaw);
				if (!!keyIter)
				{
					pvAdd(keyIter, mixedRaw.raw);
					mKeyIteratorAdd = keyIter;
				}
				else
				{
					auto keyCreator = [&mixedRaw] (Raw** newRaw) noexcept
						{ *newRaw = mixedRaw.raw; };
					mKeyIteratorAdd = mHashMultiMap.AddKeyCrt(keyIter, keyCreator);
				}
			}

			void RejectAdd() noexcept
			{
				if (!mKeyIteratorAdd)
					return;
				size_t rawCount = mKeyIteratorAdd->GetCount();
				if (rawCount > 0)
					mHashMultiMap.Remove(mKeyIteratorAdd, rawCount - 1);
				else
					mHashMultiMap.RemoveKey(mKeyIteratorAdd);
				mKeyIteratorAdd = ConstKeyIterator();
			}

			void AcceptAdd() noexcept
			{
				mKeyIteratorAdd = ConstKeyIterator();
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
				auto raws = *mKeyIteratorRemove;
				size_t rawCount = raws.GetCount();
				if (rawCount == 0)
				{
					MOMO_ASSERT(mKeyIteratorRemove->key == raw);
					mHashMultiMap.RemoveKey(mKeyIteratorRemove);
				}
				else if (mKeyIteratorRemove->key == raw)
				{
					mHashMultiMap.ResetKey(mKeyIteratorRemove, raws[rawCount - 1]);
					mHashMultiMap.Remove(mKeyIteratorRemove, rawCount - 1);
				}
				else
				{
					auto rawBegin = raws.GetBegin();
					size_t rawIndex1 = 0;
					size_t rawIndex2 = 1 << logInitialSegmentSize;
					for (size_t segIndex = 0; rawIndex2 < rawCount; ++segIndex)
					{
						size_t rawIndex = UIntMath<>::Dist(rawBegin,
							std::lower_bound(rawBegin + rawIndex1, rawBegin + rawIndex2 - 1, raw));
						if (raws[rawIndex] == raw)
						{
							std::copy(rawBegin + rawIndex + 1, rawBegin + rawIndex2, rawBegin + rawIndex);
							rawIndex = UIntMath<>::Dist(rawBegin, std::lower_bound(rawBegin + rawIndex1,
								rawBegin + rawIndex2 - 1, raws[rawCount - 1]));
							std::copy_backward(rawBegin + rawIndex, rawBegin + rawIndex2 - 1,
								rawBegin + rawIndex2);
							raws[rawIndex] = raws[rawCount - 1];
							mHashMultiMap.Remove(mKeyIteratorRemove, rawCount - 1);
							break;
						}
						rawIndex1 = rawIndex2;
						rawIndex2 += SegmentedArraySettings::GetSegmentItemCount(segIndex + 1);
					}
					if (rawIndex2 >= rawCount)
					{
						size_t rawIndex = UIntMath<>::Dist(rawBegin,
							std::find(rawBegin + rawIndex1, rawBegin + rawCount, raw));
						MOMO_ASSERT(raws[rawIndex] == raw);
						mHashMultiMap.Remove(mKeyIteratorRemove, rawIndex);
					}
				}
				mKeyIteratorRemove = KeyIterator();
			}

			template<internal::conceptPredicate<Raw*> RawFilter>
			void FilterRaws(FastCopyableFunctor<RawFilter> rawFilter) noexcept
			{
				KeyIterator keyIter = mHashMultiMap.GetKeyBounds().GetBegin();
				while (!!keyIter)
				{
					size_t rawIndex = 0;
					while (rawIndex < keyIter->GetCount())
					{
						if (rawFilter((*keyIter)[rawIndex]))
							++rawIndex;
						else
							mHashMultiMap.Remove(keyIter, rawIndex);
					}
					size_t rawCount = keyIter->GetCount();
					size_t rawIndex1 = 0;
					size_t rawIndex2 = 1 << logInitialSegmentSize;
					for (size_t segIndex = 0; rawIndex2 < rawCount; ++segIndex)
					{
						pvSortRaws(keyIter, rawIndex1, rawIndex2);
						rawIndex1 = rawIndex2;
						rawIndex2 += SegmentedArraySettings::GetSegmentItemCount(segIndex + 1);
					}
					if (rawFilter(keyIter->key))
					{
						++keyIter;
						continue;
					}
					if (rawCount == 0)
					{
						keyIter = mHashMultiMap.RemoveKey(keyIter);
						continue;
					}
					mHashMultiMap.ResetKey(keyIter, (*keyIter)[rawCount - 1]);
					mHashMultiMap.Remove(keyIter, rawCount - 1);
					++keyIter;
				}
			}

		private:
			void pvAdd(KeyIterator keyIter, Raw* raw)
			{
				size_t rawCount = keyIter->GetCount();
				if (rawCount > 0 && (rawCount & ((1 << logInitialSegmentSize) - 1)) == 0)
				{
					size_t segIndex, rawIndex;
					SegmentedArraySettings::GetSegmentItemIndexes(rawCount, segIndex, rawIndex);
					if (rawIndex == 0)
					{
						size_t segSize = SegmentedArraySettings::GetSegmentItemCount(segIndex - 1);
						pvSortRaws(keyIter, rawCount - segSize, rawCount);
					}
				}
				mHashMultiMap.Add(keyIter, raw);
			}

			static void pvSortRaws(KeyIterator keyIter, size_t rawIndex1, size_t rawIndex2) noexcept
			{
				auto rawBegin = keyIter->GetBegin();
				if (!std::is_sorted(rawBegin + rawIndex1, rawBegin + rawIndex2))
					RadixSorter<>::Sort(rawBegin + rawIndex1, rawIndex2 - rawIndex1);
			}

		private:
			Offsets mSortedOffsets;
			HashMultiMap mHashMultiMap;
			ConstKeyIterator mKeyIteratorAdd;
			KeyIterator mKeyIteratorRemove;
		};

		template<typename Hash>
		using Hashes = Array<Hash, MemManagerPtr, ArrayItemTraits<Hash, MemManagerPtr>,
			NestedArraySettings<ArraySettings<0, false>>>;

		typedef Hashes<UniqueHash> UniqueHashes;
		typedef Hashes<MultiHash> MultiHashes;

	public:
		typedef typename UniqueHash::RawBounds UniqueHashRawBounds;
		typedef typename MultiHash::RawBounds MultiHashRawBounds;

		struct Result
		{
			Raw* raw;
			UniqueHashIndex uniqueHashIndex;
		};

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

		~DataIndexes() noexcept = default;

		DataIndexes& operator=(const DataIndexes&) = delete;

		void Swap(DataIndexes& indexes) noexcept
		{
			mUniqueHashes.Swap(indexes.mUniqueHashes);
			mMultiHashes.Swap(indexes.mMultiHashes);
		}

		template<size_t columnCount>
		UniqueHashIndex GetUniqueHashIndex(const std::array<size_t, columnCount>& offsets) const
		{
			return pvGetHashIndex(mUniqueHashes, GetSortedOffsets(offsets));
		}

		template<size_t columnCount>
		MultiHashIndex GetMultiHashIndex(const std::array<size_t, columnCount>& offsets) const
		{
			return pvGetHashIndex(mMultiHashes, GetSortedOffsets(offsets));
		}

		template<typename... Items, typename Raws,
			size_t columnCount = sizeof...(Items)>
		Result AddUniqueHashIndex(const Raws& raws, const std::array<size_t, columnCount>& offsets)
		{
			Raw* resRaw = nullptr;
			auto rawAdder = [&resRaw] (UniqueHash& uniqueHash, Raw* raw)
			{
				if (uniqueHash.Add(raw) != raw)
				{
					resRaw = raw;
					return false;
				}
				return true;
			};
			UniqueHashIndex uniqueHashIndex = pvAddHashIndex<Items...>(mUniqueHashes,
				raws, offsets, FastCopyableFunctor(rawAdder));
			MOMO_ASSERT((uniqueHashIndex == UniqueHashIndex::empty) == (resRaw != nullptr));
			return { resRaw, uniqueHashIndex };
		}

		template<typename... Items, typename Raws,
			size_t columnCount = sizeof...(Items)>
		MultiHashIndex AddMultiHashIndex(const Raws& raws,
			const std::array<size_t, columnCount>& offsets)
		{
			auto rawAdder = [] (MultiHash& multiHash, Raw* raw)
			{
				multiHash.Add(raw);
				return true;
			};
			return pvAddHashIndex<Items...>(mMultiHashes,
				raws, offsets, FastCopyableFunctor(rawAdder));
		}

		void RemoveUniqueHashIndexes() noexcept
		{
			mUniqueHashes.Clear(true);
		}

		void RemoveMultiHashIndexes() noexcept
		{
			mMultiHashes.Clear(true);
		}

		void Assign(const DataIndexes& indexes)
		{
			MOMO_ASSERT(mUniqueHashes.IsEmpty());
			MOMO_ASSERT(mMultiHashes.IsEmpty());
			const MemManagerPtr& memManagerPtr = mUniqueHashes.GetMemManager();
			auto indexRemover = [this] () noexcept
			{
				RemoveUniqueHashIndexes();
				RemoveMultiHashIndexes();
			};
			for (Finalizer fin = indexRemover; fin; fin.Detach())
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
		}

		UniqueHashRawBounds FindRaws(UniqueHashIndex uniqueHashIndex, Raw* raw,
			VersionKeeper version) const
		{
			const UniqueHash& uniqueHash = pvGetHash(mUniqueHashes, uniqueHashIndex);
			return uniqueHash.Find(raw, version);	//?
		}

		template<typename... Items>
		UniqueHashRawBounds FindRaws(UniqueHashIndex uniqueHashIndex,
			const OffsetItemTuple<Items...>& tuple, VersionKeeper version) const
		{
			const UniqueHash& uniqueHash = pvGetHash(mUniqueHashes, uniqueHashIndex);
			return uniqueHash.Find(tuple, version);
		}

		template<typename... Items>
		MultiHashRawBounds FindRaws(MultiHashIndex multiHashIndex,
			const OffsetItemTuple<Items...>& tuple, VersionKeeper version) const
		{
			const MultiHash& multiHash = pvGetHash(mMultiHashes, multiHashIndex);
			return multiHash.Find(tuple, version);
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

		Result AddRaw(Raw* raw)
		{
			auto addRejector = [this] () noexcept
			{
				for (UniqueHash& uniqueHash : mUniqueHashes)
					uniqueHash.RejectAdd();
				for (MultiHash& multiHash : mMultiHashes)
					multiHash.RejectAdd();
			};
			for (Finalizer fin = addRejector; fin; fin.Detach())
			{
				for (UniqueHash& uniqueHash : mUniqueHashes)
				{
					Raw* resRaw = uniqueHash.Add(raw);
					if (resRaw != raw)	// addRejector();
						return { resRaw, pvGetHashIndex(mUniqueHashes, uniqueHash) };
				}
				for (MultiHash& multiHash : mMultiHashes)
					multiHash.Add(raw);
			}
			for (UniqueHash& uniqueHash : mUniqueHashes)
				uniqueHash.AcceptAdd();
			for (MultiHash& multiHash : mMultiHashes)
				multiHash.AcceptAdd();
			return { nullptr, UniqueHashIndex::empty };
		}

		void RemoveRaw(Raw* raw)
		{
			auto removeRejector = [this] () noexcept
			{
				for (UniqueHash& uniqueHash : mUniqueHashes)
					uniqueHash.RejectRemove();
				for (MultiHash& multiHash : mMultiHashes)
					multiHash.RejectRemove();
			};
			for (Finalizer fin = removeRejector; fin; fin.Detach())
			{
				for (UniqueHash& uniqueHash : mUniqueHashes)
					uniqueHash.PrepareRemove(raw);
				for (MultiHash& multiHash : mMultiHashes)
					multiHash.PrepareRemove(raw);
			}
			for (UniqueHash& uniqueHash : mUniqueHashes)
				uniqueHash.AcceptRemove();
			for (MultiHash& multiHash : mMultiHashes)
				multiHash.AcceptRemove(raw);
		}

		Result UpdateRaw(Raw* oldRaw, Raw* newRaw)
		{
			auto updateRejector = [this, newRaw] () noexcept
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
			};
			for (Finalizer fin = updateRejector; fin; fin.Detach())
			{
				for (UniqueHash& uniqueHash : mUniqueHashes)
				{
					Raw* resRaw = uniqueHash.Add(newRaw, oldRaw);
					if (resRaw != newRaw && resRaw != oldRaw)	// updateRejector();
						return { resRaw, pvGetHashIndex(mUniqueHashes, uniqueHash) };
					if (resRaw == newRaw)
						uniqueHash.PrepareRemove(oldRaw);
				}
				for (MultiHash& multiHash : mMultiHashes)
				{
					multiHash.Add(newRaw);
					multiHash.PrepareRemove(oldRaw);
				}
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
			return { nullptr, UniqueHashIndex::empty };
		}

		template<typename Item, conceptMoveFunctor<void, Raw*, size_t> ItemAssigner>
		Result UpdateRaw(Raw* raw, size_t offset, const Item& item,
			FastMovableFunctor<ItemAssigner> itemAssigner)
		{
			if (DataTraits::IsEqual(item, ColumnList::template GetByOffset<const Item>(raw, offset)))
			{
				std::move(itemAssigner)(raw, offset);
				return { nullptr, UniqueHashIndex::empty };
			}
			MixedRaw<Item> mixedRaw{ raw, offset, std::addressof(item) };
			auto updateRejector = [this] () noexcept
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
			};
			for (Finalizer fin = updateRejector; fin; fin.Detach())
			{
				for (UniqueHash& uniqueHash : mUniqueHashes)
				{
					if (!pvContainsOffset(uniqueHash, offset))
						continue;
					Raw* resRaw = uniqueHash.Add(mixedRaw);
					if (resRaw != raw)	// updateRejector();
						return { resRaw, pvGetHashIndex(mUniqueHashes, uniqueHash) };
					uniqueHash.PrepareRemove(raw);
				}
				for (MultiHash& multiHash : mMultiHashes)
				{
					if (!pvContainsOffset(multiHash, offset))
						continue;
					multiHash.Add(mixedRaw);
					multiHash.PrepareRemove(raw);
				}
				std::move(itemAssigner)(raw, offset);
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
			return { nullptr, UniqueHashIndex::empty };
		}

		template<internal::conceptPredicate<Raw*> RawFilter>
		void FilterRaws(FastCopyableFunctor<RawFilter> rawFilter) noexcept
		{
			for (UniqueHash& uniqueHash : mUniqueHashes)
				uniqueHash.FilterRaws(rawFilter);
			for (MultiHash& multiHash : mMultiHashes)
				multiHash.FilterRaws(rawFilter);
		}

		template<size_t columnCount>
		static std::array<size_t, columnCount> GetSortedOffsets(
			const std::array<size_t, columnCount>& offsets)
		{
			static_assert(columnCount > 0);
			std::array<size_t, columnCount> sortedOffsets = offsets;
			RadixSorter<>::Sort(sortedOffsets.data(), columnCount);
			MOMO_CHECK(std::unique(sortedOffsets.begin(), sortedOffsets.end()) == sortedOffsets.end());
			return sortedOffsets;
		}

		template<size_t columnCount>
		UniqueHashIndex GetFitUniqueHashIndex(
			const std::array<size_t, columnCount>& sortedOffsets) const noexcept
		{
			for (const UniqueHash& uniqueHash : mUniqueHashes)
			{
				const Offsets& hashSortedOffsets = uniqueHash.GetSortedOffsets();
				bool includes = std::includes(sortedOffsets.begin(), sortedOffsets.end(),
					hashSortedOffsets.GetBegin(), hashSortedOffsets.GetEnd());
				if (includes)
					return pvGetHashIndex(mUniqueHashes, uniqueHash);
			}
			return UniqueHashIndex::empty;
		}

		template<size_t columnCount>
		MultiHashIndex GetFitMultiHashIndex(
			const std::array<size_t, columnCount>& sortedOffsets) const noexcept
		{
			MultiHashIndex multiHashIndex = MultiHashIndex::empty;
			size_t maxKeyCount = 0;
			for (const MultiHash& multiHash : mMultiHashes)
			{
				const Offsets& hashSortedOffsets = multiHash.GetSortedOffsets();
				bool includes = std::includes(sortedOffsets.begin(), sortedOffsets.end(),
					hashSortedOffsets.GetBegin(), hashSortedOffsets.GetEnd());
				size_t keyCount = multiHash.GetKeyCount();
				if (includes && keyCount > maxKeyCount)
				{
					maxKeyCount = keyCount;
					multiHashIndex = pvGetHashIndex(mMultiHashes, multiHash);
				}
			}
			return multiHashIndex;
		}

		template<size_t columnCount>
		UniqueHashIndex GetTrueIndex(UniqueHashIndex uniqueHashIndex,
			const std::array<size_t, columnCount>& offsets) const
		{
			return pvGetTrueIndex(mUniqueHashes, uniqueHashIndex, offsets);
		}

		template<size_t columnCount>
		MultiHashIndex GetTrueIndex(MultiHashIndex multiHashIndex,
			const std::array<size_t, columnCount>& offsets) const
		{
			return pvGetTrueIndex(mMultiHashes, multiHashIndex, offsets);
		}

		bool ContainsOffset(UniqueHashIndex uniqueHashIndex, size_t offset) const noexcept
		{
			return pvContainsOffset(pvGetHash(mUniqueHashes, uniqueHashIndex), offset);
		}

		bool ContainsOffset(MultiHashIndex multiHashIndex, size_t offset) const noexcept
		{
			return pvContainsOffset(pvGetHash(mMultiHashes, multiHashIndex), offset);
		}

	private:
		template<typename Hash,
			typename Index = typename Hash::Index>
		static const Hash& pvGetHash(const Hashes<Hash>& hashes, Index index) noexcept
		{
			MOMO_ASSERT(index != Index::empty);
			return hashes[static_cast<size_t>(index)];
		}

		template<typename Hash,
			typename Index = typename Hash::Index>
		static Index pvGetHashIndex(const Hashes<Hash>& hashes, const Hash& hash) noexcept
		{
			return static_cast<Index>(&hash - hashes.GetItems());
		}

		template<typename Hash, size_t columnCount,
			typename Index = typename Hash::Index>
		static Index pvGetHashIndex(const Hashes<Hash>& hashes,
			const std::array<size_t, columnCount>& sortedOffsets) noexcept
		{
			for (const Hash& hash : hashes)
			{
				const Offsets& hashSortedOffsets = hash.GetSortedOffsets();
				bool equal = hashSortedOffsets.GetCount() == columnCount
					&& std::equal(sortedOffsets.begin(), sortedOffsets.end(),
					hashSortedOffsets.GetBegin());
				if (equal)
					return pvGetHashIndex(hashes, hash);
			}
			return Index::empty;
		}

		template<typename... Items, typename Hash, typename Raws,
			conceptConstFunctor<void, Hash&, Raw*> RawAdder,
			size_t columnCount = sizeof...(Items),
			typename Index = typename Hash::Index>
		static Index pvAddHashIndex(Hashes<Hash>& hashes, const Raws& raws,
			const std::array<size_t, columnCount>& offsets, FastCopyableFunctor<RawAdder> rawAdder)
		{
			std::array<size_t, columnCount> sortedOffsets = GetSortedOffsets(offsets);
			Index index = pvGetHashIndex(hashes, sortedOffsets);
			if (index != Index::empty)
				return index;
			auto rawHasher = [] (Raw* key, const size_t* offsets)
			{
				size_t hashCode = 0;
				const size_t* offsetPtr = offsets;
				(pvAccumulateHashCode<Items>(hashCode, key, *offsetPtr++), ...);
				return hashCode;
			};
			auto rawEqualComp = [] (Raw* key1, Raw* key2, const size_t* offsets)
			{
				const size_t* offsetPtr = offsets;
				return (pvIsEqual<Items>(key1, key2, *offsetPtr++) && ...);
			};
			auto mixedRawHasher = [] (const MixedRaw<>& key, const size_t* offsets)
			{
				size_t hashCode = 0;
				const size_t* offsetPtr = offsets;
				(pvAccumulateHashCode<Items>(hashCode, key, *offsetPtr++), ...);
				return hashCode;
			};
			auto mixedRawEqualComp = [] (const MixedRaw<>& key1, Raw* key2, const size_t* offsets)
			{
				const size_t* offsetPtr = offsets;
				return (pvIsEqual<Items>(key1, key2, *offsetPtr++) && ...);
			};
			const MemManagerPtr& memManagerPtr = hashes.GetMemManager();
			HashTraits hashTraits(rawHasher, rawEqualComp, mixedRawHasher, mixedRawEqualComp,
				Offsets(offsets.begin(), offsets.end(), MemManagerPtr(memManagerPtr)));
			Hash hash(std::move(hashTraits),
				Offsets(sortedOffsets.begin(), sortedOffsets.end(), MemManagerPtr(memManagerPtr)));
			for (Raw* raw : raws)
			{
				if (!rawAdder(hash, raw))
					return Index::empty;
				hash.AcceptAdd();
			}
			hashes.Reserve(hashes.GetCount() + 1);
			hashes.AddBackNogrow(std::move(hash));
			return pvGetHashIndex(hashes, hashes.GetBackItem());
		}

		template<typename Hash, typename Index, size_t columnCount>
		static Index pvGetTrueIndex(const Hashes<Hash>& hashes, Index index,
			const std::array<size_t, columnCount>& offsets)
		{
			if (index != Index::empty)
			{
				MOMO_EXTRA_CHECK(index == pvGetHashIndex(hashes, GetSortedOffsets(offsets)));
				return index;
			}
			Index trueIndex = pvGetHashIndex(hashes, GetSortedOffsets(offsets));
			if (trueIndex == Index::empty)
				MOMO_THROW(std::logic_error("Index not found"));
			return trueIndex;
		}

		template<typename Hash>
		static bool pvContainsOffset(const Hash& hash, size_t offset) noexcept
		{
			const Offsets& sortedOffsets = hash.GetSortedOffsets();
			return sortedOffsets.Contains(offset);
			//return std::binary_search(sortedOffsets.GetBegin(), sortedOffsets.GetEnd(), offset);
		}

		template<typename Item>
		static void pvAccumulateHashCode(size_t& hashCode, Raw* key, size_t offset)
		{
			const Item& item = ColumnList::template GetByOffset<const Item>(key, offset);
			DataTraits::AccumulateHashCode(hashCode, item, offset);
		}

		template<typename Item>
		static void pvAccumulateHashCode(size_t& hashCode, const MixedRaw<>& key, size_t offset)
		{
			const Item& item = (offset != key.offset)
				? ColumnList::template GetByOffset<const Item>(key.raw, offset)
				: *static_cast<const Item*>(key.item);
			DataTraits::AccumulateHashCode(hashCode, item, offset);
		}

		template<typename Item>
		static bool pvIsEqual(Raw* key1, Raw* key2, size_t offset)
		{
			const Item& item1 = ColumnList::template GetByOffset<const Item>(key1, offset);
			const Item& item2 = ColumnList::template GetByOffset<const Item>(key2, offset);
			return DataTraits::IsEqual(item1, item2);
		}

		template<typename Item>
		static bool pvIsEqual(const MixedRaw<>& key1, Raw* key2, size_t offset)
		{
			const Item& item1 = (offset != key1.offset)
				? ColumnList::template GetByOffset<const Item>(key1.raw, offset)
				: *static_cast<const Item*>(key1.item);
			const Item& item2 = ColumnList::template GetByOffset<const Item>(key2, offset);
			return DataTraits::IsEqual(item1, item2);
		}

	private:
		UniqueHashes mUniqueHashes;
		MultiHashes mMultiHashes;
	};
}

} // namespace momo

namespace std
{
	template<typename R, typename S>
	struct iterator_traits<momo::internal::DataRawUniqueHashIterator<R, S>>
		: public momo::internal::IteratorTraitsStd<momo::internal::DataRawUniqueHashIterator<R, S>,
			random_access_iterator_tag>
	{
	};

	template<typename RI, typename S>
	struct iterator_traits<momo::internal::DataRawMultiHashIterator<RI, S>>
		: public momo::internal::IteratorTraitsStd<momo::internal::DataRawMultiHashIterator<RI, S>,
			random_access_iterator_tag>
	{
	};
} // namespace std

/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/HashSet.h

  namespace momo:
    class HashSetItemTraits
    class HashSetSettings
    class HashSet
    class HashSetOpen

  All `HashSet` functions and constructors have strong exception safety,
  but not the following cases:
  1. Functions `Insert` receiving many items have basic exception safety.
  2. Functions `MergeFrom` and `MergeTo` have basic exception safety.
  3. If constructor receiving many items throws exception, input argument
    `memManager` may be changed.

\**********************************************************/

#pragma once

#include "HashTraits.h"
#include "SetUtility.h"
#include "IteratorUtility.h"

namespace momo
{

namespace internal
{
	template<typename TBucket>
	class HashSetBuckets
	{
	public:
		typedef TBucket Bucket;
		typedef typename Bucket::MemManager MemManager;
		typedef typename Bucket::Params BucketParams;

		static const size_t maxBucketCount =
			(SIZE_MAX - sizeof(size_t) - 2 * sizeof(void*)) / sizeof(Bucket);

	private:
		typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

	public:
		HashSetBuckets() = delete;

		HashSetBuckets(const HashSetBuckets&) = delete;

		~HashSetBuckets() = delete;

		HashSetBuckets& operator=(const HashSetBuckets&) = delete;

		static HashSetBuckets* Create(MemManager& memManager, size_t logBucketCount,
			BucketParams* bucketParams)
		{
			size_t bucketCount = (size_t)1 << logBucketCount;
			if (bucketCount > maxBucketCount)
				throw std::length_error("momo::internal::HashSetBuckets length error");
			size_t bufferSize = pvGetBufferSize(logBucketCount);
			HashSetBuckets* resBuckets = MemManagerProxy::template Allocate<HashSetBuckets>(
				memManager, bufferSize);
			resBuckets->mLogCount = logBucketCount;
			resBuckets->mNextBuckets = nullptr;
			Bucket* buckets = resBuckets->pvGetBuckets();
			size_t bucketIndex = 0;
			try
			{
				for (; bucketIndex < bucketCount; ++bucketIndex)
					::new(static_cast<void*>(buckets + bucketIndex)) Bucket();
				if (bucketParams == nullptr)
					resBuckets->mBucketParams = pvCreateBucketParams(memManager);
				else
					resBuckets->mBucketParams = bucketParams;
			}
			catch (...)
			{
				for (size_t i = 0; i < bucketIndex; ++i)
					buckets[i].~Bucket();
				MemManagerProxy::Deallocate(memManager, resBuckets, bufferSize);
				throw;
			}
			return resBuckets;
		}

		void Destroy(MemManager& memManager, bool destroyBucketParams) noexcept
		{
			MOMO_ASSERT(mNextBuckets == nullptr);
			size_t bucketCount = GetCount();
			Bucket* buckets = pvGetBuckets();
			for (size_t i = 0; i < bucketCount; ++i)
				buckets[i].~Bucket();
			if (destroyBucketParams)
			{
				mBucketParams->~BucketParams();
				MemManagerProxy::Deallocate(memManager, mBucketParams, sizeof(BucketParams));
			}
			MemManagerProxy::Deallocate(memManager, this, pvGetBufferSize(GetLogCount()));
		}

		Bucket* GetBegin() noexcept
		{
			return pvGetBuckets();
		}

		Bucket* GetEnd() noexcept
		{
			return pvGetBuckets() + GetCount();
		}

		MOMO_FRIENDS_BEGIN_END(HashSetBuckets&, Bucket*)

		HashSetBuckets* GetNextBuckets() noexcept
		{
			return mNextBuckets;
		}

		HashSetBuckets* ExtractNextBuckets() noexcept
		{
			HashSetBuckets* nextBuckets = mNextBuckets;
			mNextBuckets = nullptr;
			return nextBuckets;
		}

		void SetNextBuckets(HashSetBuckets* nextBuckets) noexcept
		{
			MOMO_ASSERT(mNextBuckets == nullptr);
			mNextBuckets = nextBuckets;
		}

		size_t GetCount() const noexcept
		{
			return (size_t)1 << mLogCount;
		}

		size_t GetLogCount() const noexcept
		{
			return mLogCount;
		}

		Bucket& operator[](size_t index) noexcept
		{
			MOMO_ASSERT(index < GetCount());
			return pvGetBuckets()[index];
		}

		BucketParams& GetBucketParams() noexcept
		{
			return *mBucketParams;
		}

	private:
		Bucket* pvGetBuckets() noexcept
		{
			return BitCaster::PtrToPtr<Bucket>(this, sizeof(HashSetBuckets));
		}

		static size_t pvGetBufferSize(size_t logBucketCount) noexcept
		{
			return sizeof(HashSetBuckets) + (sizeof(Bucket) << logBucketCount);
		}

		static BucketParams* pvCreateBucketParams(MemManager& memManager)
		{
			BucketParams* bucketParams = MemManagerProxy::template Allocate<BucketParams>(
				memManager, sizeof(BucketParams));
			try
			{
				::new(static_cast<void*>(bucketParams)) BucketParams(memManager);
			}
			catch (...)
			{
				MemManagerProxy::Deallocate(memManager, bucketParams, sizeof(BucketParams));
				throw;
			}
			return bucketParams;
		}

	private:
		size_t mLogCount;
		HashSetBuckets* mNextBuckets;
		union
		{
			BucketParams* mBucketParams;
			typename std::aligned_storage<alignof(Bucket), alignof(Bucket)>::type mBucketPadding;
		};
	};

	template<typename TBucket, typename TSettings>
	class HashSetConstIterator;

	template<typename TBucket, typename TSettings>
	class HashSetConstPosition : private VersionKeeper<TSettings>
	{
	protected:
		typedef TBucket Bucket;
		typedef TSettings Settings;

		typedef typename Bucket::Iterator BucketIterator;

	private:
		typedef typename Bucket::Item Item;

		typedef internal::VersionKeeper<Settings> VersionKeeper;

	public:
		typedef const Item& Reference;
		typedef const Item* Pointer;

		typedef HashSetConstPosition ConstPosition;

		typedef HashSetConstIterator<Bucket, Settings> Iterator;

	public:
		explicit HashSetConstPosition() noexcept
			: mBucketIterator(),
			mIndexCode(0)
		{
		}

		//operator ConstPosition() const noexcept

		Pointer operator->() const
		{
			VersionKeeper::Check();
			MOMO_CHECK(mBucketIterator != BucketIterator());
			return std::addressof(*mBucketIterator);
		}

		bool operator==(ConstPosition pos) const noexcept
		{
			return mBucketIterator == pos.mBucketIterator;
		}

		MOMO_MORE_HASH_POSITION_OPERATORS(HashSetConstPosition)

	protected:
		explicit HashSetConstPosition(size_t indexCode, BucketIterator bucketIter,
			const size_t* version) noexcept
			: VersionKeeper(version),
			mBucketIterator(bucketIter),
			mIndexCode(indexCode)
		{
		}

		size_t ptGetBucketIndex() const noexcept
		{
			//MOMO_ASSERT(mBucketIterator != BucketIterator());	//?
			return mIndexCode;
		}

		size_t ptGetHashCode() const noexcept
		{
			MOMO_ASSERT(mBucketIterator == BucketIterator());
			return mIndexCode;
		}

		BucketIterator ptGetBucketIterator() const noexcept
		{
			return mBucketIterator;
		}

		void ptReset(size_t bucketIndex, BucketIterator bucketIter) noexcept
		{
			mIndexCode = bucketIndex;
			mBucketIterator = bucketIter;
		}

		void ptCheck(const size_t* version, bool allowEmpty) const
		{
			VersionKeeper::Check(version, allowEmpty);
		}

	private:
		BucketIterator mBucketIterator;
		size_t mIndexCode;
	};

	template<typename TBucket, typename TSettings>
	class HashSetConstIterator : public HashSetConstPosition<TBucket, TSettings>
	{
	public:
		typedef HashSetConstPosition<TBucket, TSettings> Position;

	protected:
		using typename Position::Bucket;
		using typename Position::Settings;

		using typename Position::BucketIterator;

		typedef HashSetBuckets<Bucket> Buckets;

	public:
		using typename Position::Reference;
		using typename Position::Pointer;

		typedef HashSetConstIterator ConstIterator;

	private:
		//using typename Position::ConstPosition;
		using typename Position::Iterator;	//?

		typedef typename Bucket::Bounds BucketBounds;

	public:
		explicit HashSetConstIterator() noexcept
			: mBuckets(nullptr)
		{
		}

		HashSetConstIterator(Position pos) noexcept
			: Position(pos),
			mBuckets(nullptr)
		{
		}

		//operator ConstIterator() const noexcept

		HashSetConstIterator& operator++()
		{
			Position::operator->();	// check
			if (ptIsMovable())
				pvInc();
			else
				*this = HashSetConstIterator();
			return *this;
		}

		//MOMO_MORE_HASH_ITERATOR_OPERATORS(HashSetConstIterator)
		HashSetConstIterator operator++(int)
		{
			HashSetConstIterator tempIter = *this;
			++*this;
			return tempIter;
		}

	protected:
		explicit HashSetConstIterator(Buckets& buckets, size_t bucketIndex,
			BucketIterator bucketIter, const size_t* version) noexcept
			: Position(bucketIndex, bucketIter, version),
			mBuckets(&buckets)
		{
			pvInc();
		}

		bool ptIsMovable() const noexcept
		{
			MOMO_ASSERT(Position::ptGetBucketIterator() != BucketIterator());
			return mBuckets != nullptr;
		}

	private:
		void pvInc() noexcept
		{
			BucketIterator bucketIter = Position::ptGetBucketIterator();
			size_t bucketIndex = Position::ptGetBucketIndex();
			if (bucketIter != pvGetBucketBounds(bucketIndex).GetBegin())
				Position::ptReset(bucketIndex, std::prev(bucketIter));
			else
				pvMove();
		}

		void pvMove() noexcept
		{
			size_t bucketCount = mBuckets->GetCount();
			size_t bucketIndex = Position::ptGetBucketIndex();
			while (true)
			{
				++bucketIndex;
				if (bucketIndex >= bucketCount)
					break;
				BucketBounds bounds = pvGetBucketBounds(bucketIndex);
				if (bounds.GetCount() > 0)
				{
					Position::ptReset(bucketIndex, std::prev(bounds.GetEnd()));
					return;
				}
			}
			Buckets* nextBuckets = mBuckets->GetNextBuckets();
			if (nextBuckets != nullptr)
			{
				mBuckets = nextBuckets;
				Position::ptReset(0, pvGetBucketBounds(0).GetEnd());
				return pvInc();	//?
			}
			*this = HashSetConstIterator();
		}

		BucketBounds pvGetBucketBounds(size_t bucketIndex) const noexcept
		{
			return (*mBuckets)[bucketIndex].GetBounds(mBuckets->GetBucketParams());
		}

	private:
		Buckets* mBuckets;
	};

	template<typename THashSetItemTraits>
	class HashSetBucketItemTraits
	{
	protected:
		typedef THashSetItemTraits HashSetItemTraits;

	public:
		typedef typename HashSetItemTraits::Item Item;
		typedef typename HashSetItemTraits::MemManager MemManager;

		static const size_t alignment = HashSetItemTraits::alignment;

	public:
		static void Destroy(MemManager& memManager, Item* items, size_t count) noexcept
		{
			for (size_t i = 0; i < count; ++i)
				HashSetItemTraits::Destroy(&memManager, items[i]);
		}

		template<typename ItemCreator>
		static void RelocateCreate(MemManager& memManager, Item* srcItems, Item* dstItems,
			size_t count, ItemCreator&& itemCreator, Item* newItem)
		{
			HashSetItemTraits::RelocateCreate(memManager, srcItems, dstItems, count,
				std::forward<ItemCreator>(itemCreator), newItem);
		}
	};
}

template<typename TKey, typename TItem, typename TMemManager>
class HashSetItemTraits : public internal::SetItemTraits<TKey, TItem, TMemManager>
{
private:
	typedef internal::SetItemTraits<TKey, TItem, TMemManager> SetItemTraits;

public:
	using typename SetItemTraits::Item;
	using typename SetItemTraits::MemManager;

private:
	typedef internal::ObjectManager<Item, MemManager> ItemManager;

public:
	template<typename ItemCreator>
	static void RelocateCreate(MemManager& memManager, Item* srcItems, Item* dstItems,
		size_t count, ItemCreator&& itemCreator, Item* newItem)
	{
		ItemManager::RelocateCreate(memManager, srcItems, dstItems, count,
			std::forward<ItemCreator>(itemCreator), newItem);
	}
};

class HashSetSettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION;

	static const bool overloadIfCannotGrow = true;
};

template<typename TKey,
	typename THashTraits = HashTraits<TKey>,
	typename TMemManager = MemManagerDefault,
	typename TItemTraits = HashSetItemTraits<TKey, TKey, TMemManager>,
	typename TSettings = HashSetSettings>
class HashSet
{
public:
	typedef TKey Key;
	typedef THashTraits HashTraits;
	typedef TMemManager MemManager;
	typedef TItemTraits ItemTraits;
	typedef TSettings Settings;
	typedef typename ItemTraits::Item Item;

private:
	typedef internal::SetCrew<HashTraits, MemManager, Settings::checkVersion> Crew;

	typedef internal::HashSetBucketItemTraits<ItemTraits> BucketItemTraits;

	static const bool useHashCodePartGetter = !HashTraits::isFastNothrowHashable;

	typedef typename HashTraits::HashBucket HashBucket;
	typedef typename HashBucket::template Bucket<BucketItemTraits, useHashCodePartGetter> Bucket;

	typedef typename Bucket::Params BucketParams;

	typedef typename Bucket::Iterator BucketIterator;
	typedef typename Bucket::Bounds BucketBounds;

	typedef internal::HashSetBuckets<Bucket> Buckets;

public:
	typedef internal::HashSetConstIterator<Bucket, Settings> ConstIterator;
	typedef ConstIterator Iterator;	//?

	typedef typename ConstIterator::Position ConstPosition;
	typedef ConstPosition Position;

	typedef internal::InsertResult<ConstPosition> InsertResult;

	typedef internal::SetExtractedItem<ItemTraits, Settings> ExtractedItem;

	typedef typename BucketBounds::ConstBounds ConstBucketBounds;

	static const size_t bucketMaxItemCount = Bucket::maxCount;

private:
	static const bool areItemsNothrowRelocatable = HashTraits::isFastNothrowHashable
		&& ItemTraits::isNothrowRelocatable && Bucket::isNothrowAddableIfNothrowCreatable;

	template<typename... ItemArgs>
	using Creator = typename ItemTraits::template Creator<ItemArgs...>;

	template<typename KeyArg>
	struct IsValidKeyArg : public HashTraits::template IsValidKeyArg<KeyArg>
	{
	};

	struct ConstIteratorProxy : public ConstIterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, IsMovable, bool)
	};

	struct ConstPositionProxy : public ConstPosition
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstPosition)
		MOMO_DECLARE_PROXY_FUNCTION(ConstPosition, GetBucketIndex, size_t)
		MOMO_DECLARE_PROXY_FUNCTION(ConstPosition, GetHashCode, size_t)
		MOMO_DECLARE_PROXY_FUNCTION(ConstPosition, GetBucketIterator, BucketIterator)
		MOMO_DECLARE_PROXY_FUNCTION(ConstPosition, Reset, void)
		MOMO_DECLARE_PROXY_FUNCTION(ConstPosition, Check, void)
	};

public:
	HashSet()
		: HashSet(HashTraits())
	{
	}

	explicit HashSet(const HashTraits& hashTraits, MemManager&& memManager = MemManager())
		: mCrew(hashTraits, std::move(memManager)),
		mCount(0),
		mCapacity(0),
		mBuckets(nullptr)
	{
	}

	HashSet(std::initializer_list<Item> items,
		const HashTraits& hashTraits = HashTraits(), MemManager&& memManager = MemManager())
		: HashSet(hashTraits, std::move(memManager))
	{
		try
		{
			Insert(items);
		}
		catch (...)
		{
			pvDestroy();
			throw;
		}
	}

	HashSet(HashSet&& hashSet) noexcept
		: mCrew(std::move(hashSet.mCrew)),
		mCount(hashSet.mCount),
		mCapacity(hashSet.mCapacity),
		mBuckets(hashSet.mBuckets)
	{
		hashSet.mCount = 0;
		hashSet.mCapacity = 0;
		hashSet.mBuckets = nullptr;
	}

	HashSet(const HashSet& hashSet)
		: HashSet(hashSet, MemManager(hashSet.GetMemManager()))
	{
	}

	HashSet(const HashSet& hashSet, MemManager&& memManager)
		: HashSet(hashSet.GetHashTraits(), std::move(memManager))
	{
		mCount = hashSet.mCount;
		if (mCount == 0)
			return;
		const HashTraits& hashTraits = GetHashTraits();
		size_t logBucketCount = hashTraits.GetLogStartBucketCount();
		while (true)
		{
			mCapacity = hashTraits.CalcCapacity((size_t)1 << logBucketCount, bucketMaxItemCount);
			if (mCapacity >= mCount)
				break;
			++logBucketCount;
		}
		mBuckets = Buckets::Create(GetMemManager(), logBucketCount, nullptr);
		try
		{
			for (const Item& item : hashSet)
			{
				size_t hashCode = hashTraits.GetHashCode(ItemTraits::GetKey(item));
				pvAddNogrow<false>(*mBuckets, hashCode,
					Creator<const Item&>(GetMemManager(), item));
			}
		}
		catch (...)
		{
			pvDestroy();
			throw;
		}
	}

	~HashSet() noexcept
	{
		pvDestroy();
	}

	HashSet& operator=(HashSet&& hashSet) noexcept
	{
		HashSet(std::move(hashSet)).Swap(*this);
		return *this;
	}

	HashSet& operator=(const HashSet& hashSet)
	{
		if (this != &hashSet)
			HashSet(hashSet).Swap(*this);
		return *this;
	}

	void Swap(HashSet& hashSet) noexcept
	{
		mCrew.Swap(hashSet.mCrew);
		std::swap(mCount, hashSet.mCount);
		std::swap(mCapacity, hashSet.mCapacity);
		std::swap(mBuckets, hashSet.mBuckets);
	}

	ConstIterator GetBegin() const noexcept
	{
		if (mCount == 0)
			return ConstIterator();
		return ConstIteratorProxy(*mBuckets, 0,
			mBuckets->GetBegin()->GetBounds(mBuckets->GetBucketParams()).GetEnd(),
			mCrew.GetVersion());
	}

	ConstIterator GetEnd() const noexcept
	{
		return ConstIterator();
	}

	MOMO_FRIEND_SWAP(HashSet)
	MOMO_FRIENDS_BEGIN_END(const HashSet&, ConstIterator)

	const HashTraits& GetHashTraits() const noexcept
	{
		return mCrew.GetContainerTraits();
	}

	const MemManager& GetMemManager() const noexcept
	{
		return mCrew.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mCrew.GetMemManager();
	}

	size_t GetCount() const noexcept
	{
		return mCount;
	}

	bool IsEmpty() const noexcept
	{
		return mCount == 0;
	}

	void Clear(bool shrink = true) noexcept
	{
		if (mBuckets == nullptr)
			return;
		if (shrink)
		{
			pvDestroy();
			mBuckets = nullptr;
			mCapacity = 0;
		}
		else
		{
			pvDestroy(mBuckets->ExtractNextBuckets(), false);
			BucketParams& bucketParams = mBuckets->GetBucketParams();
			for (Bucket& bucket : *mBuckets)
				bucket.Clear(bucketParams);
		}
		mCount = 0;
		mCrew.IncVersion();
	}

	size_t GetCapacity() const noexcept
	{
		return mCapacity;
	}

	void Reserve(size_t capacity)
	{
		if (capacity <= mCapacity)
			return;
		const HashTraits& hashTraits = GetHashTraits();
		size_t newLogBucketCount = pvGetNewLogBucketCount();
		size_t newCapacity;
		while (true)
		{
			newCapacity = hashTraits.CalcCapacity((size_t)1 << newLogBucketCount,
				bucketMaxItemCount);
			if (newCapacity >= capacity)
				break;
			++newLogBucketCount;
		}
		Buckets* newBuckets = Buckets::Create(GetMemManager(), newLogBucketCount,
			(mBuckets != nullptr) ? &mBuckets->GetBucketParams() : nullptr);
		newBuckets->SetNextBuckets(mBuckets);
		mBuckets = newBuckets;
		mCapacity = newCapacity;
		mCrew.IncVersion();
		if (mBuckets->GetNextBuckets() != nullptr)
			pvRelocateItems();
	}

	void Shrink()
	{
		HashSet(*this).Swap(*this);
	}

	ConstPosition Find(const Key& key) const
	{
		return pvFind(key);
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value, ConstPosition> Find(const KeyArg& key) const
	{
		return pvFind(key);
	}

	ConstPosition Find(const Key& key, size_t hashCode) const
	{
		return pvFind(key, hashCode);
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value, ConstPosition> Find(const KeyArg& key,
		size_t hashCode) const
	{
		return pvFind(key, hashCode);
	}

	bool ContainsKey(const Key& key) const
	{
		return !!pvFind(key);
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value, bool> ContainsKey(const KeyArg& key) const
	{
		return !!pvFind(key);
	}

	template<typename ItemCreator>
	InsertResult InsertCrt(const Key& key, ItemCreator&& itemCreator)
	{
		return pvInsert<true>(key, std::forward<ItemCreator>(itemCreator));
	}

	template<typename... ItemArgs>
	InsertResult InsertVar(const Key& key, ItemArgs&&... itemArgs)
	{
		return InsertCrt(key,
			Creator<ItemArgs...>(GetMemManager(), std::forward<ItemArgs>(itemArgs)...));
	}

	InsertResult Insert(Item&& item)
	{
		const Key& key = ItemTraits::GetKey(static_cast<const Item&>(item));
		return pvInsert<false>(key, Creator<Item&&>(GetMemManager(), std::move(item)));
	}

	InsertResult Insert(const Item& item)
	{
		return pvInsert<false>(ItemTraits::GetKey(item),
			Creator<const Item&>(GetMemManager(), item));
	}

	InsertResult Insert(ExtractedItem&& extItem)
	{
		MOMO_CHECK(!extItem.IsEmpty());
		MemManager& memManager = GetMemManager();
		auto itemCreator = [&memManager, &extItem] (Item* newItem)
		{
			auto itemRemover = [&memManager, newItem] (Item& item)
				{ ItemTraits::Relocate(&memManager, item, newItem); };
			extItem.Remove(itemRemover);
		};
		return pvInsert<false>(ItemTraits::GetKey(extItem.GetItem()), itemCreator);
	}

	template<typename ArgIterator>
	size_t Insert(ArgIterator begin, ArgIterator end)
	{
		MOMO_CHECK_ITERATOR_REFERENCE(ArgIterator, Item);
		size_t count = 0;
		for (ArgIterator iter = begin; iter != end; ++iter)
			count += Insert(*iter).inserted ? 1 : 0;
		return count;
	}

	size_t Insert(std::initializer_list<Item> items)
	{
		return Insert(items.begin(), items.end());
	}

	template<typename ItemCreator, bool extraCheck = true>
	ConstPosition AddCrt(ConstPosition pos, ItemCreator&& itemCreator)
	{
		return pvAdd<extraCheck>(pos, std::forward<ItemCreator>(itemCreator));
	}

	template<typename... ItemArgs>
	ConstPosition AddVar(ConstPosition pos, ItemArgs&&... itemArgs)
	{
		return AddCrt(pos,
			Creator<ItemArgs...>(GetMemManager(), std::forward<ItemArgs>(itemArgs)...));
	}

	ConstPosition Add(ConstPosition pos, Item&& item)
	{
		return AddVar(pos, std::move(item));
	}

	ConstPosition Add(ConstPosition pos, const Item& item)
	{
		return AddVar(pos, item);
	}

	ConstPosition Add(ConstPosition pos, ExtractedItem&& extItem)
	{
		MOMO_CHECK(!extItem.IsEmpty());
		MemManager& memManager = GetMemManager();
		auto itemCreator = [&memManager, &extItem] (Item* newItem)
		{
			auto itemRemover = [&memManager, newItem] (Item& item)
				{ ItemTraits::Relocate(&memManager, item, newItem); };
			extItem.Remove(itemRemover);
		};
		return AddCrt(pos, itemCreator);
	}

	ConstIterator Remove(ConstIterator iter)
	{
		auto itemReplacer = [this] (Item& srcItem, Item& dstItem)
			{ ItemTraits::Replace(GetMemManager(), srcItem, dstItem); };
		return pvRemove(iter, itemReplacer);
	}

	ConstIterator Remove(ConstIterator iter, ExtractedItem& extItem)
	{
		MOMO_CHECK(extItem.IsEmpty());
		ConstIterator resIter;
		auto itemCreator = [this, iter, &resIter] (Item* newItem)
			{ resIter = pvExtract(iter, newItem); };
		extItem.Create(itemCreator);
		return resIter;
	}

	bool Remove(const Key& key)
	{
		ConstPosition pos = pvFind(key);
		if (!pos)
			return false;
		Remove(static_cast<ConstIterator>(pos));
		return true;
	}

	ExtractedItem Extract(ConstPosition pos)
	{
		return ExtractedItem(*this, static_cast<ConstIterator>(pos));	// need RVO for exception safety
	}

	template<typename KeyArg, bool extraCheck = true>
	void ResetKey(ConstPosition pos, KeyArg&& keyArg)
	{
		ConstPositionProxy::Check(pos, mCrew.GetVersion(), false);
		BucketIterator bucketIter = ConstPositionProxy::GetBucketIterator(pos);
		MOMO_CHECK(bucketIter != BucketIterator());
		ItemTraits::AssignKey(GetMemManager(), std::forward<KeyArg>(keyArg), *bucketIter);
		MOMO_EXTRA_CHECK(!extraCheck || pvExtraCheck(pos));
	}

	template<typename RSet>
	void MergeFrom(RSet&& srcSet)
	{
		srcSet.MergeTo(*this);
	}

	template<typename Set>
	void MergeTo(Set& dstSet)
	{
		MOMO_STATIC_ASSERT((std::is_same<Key, typename Set::Key>::value));
		MOMO_STATIC_ASSERT((std::is_same<Item, typename Set::Item>::value));
		pvMergeTo(dstSet);
	}

	void MergeTo(HashSet& dstHashSet)
	{
		if (this == &dstHashSet)
			return;
		pvMergeTo(dstHashSet);
	}

	size_t GetBucketCount() const noexcept
	{
		size_t bucketCount = 0;
		for (Buckets* bkts = mBuckets; bkts != nullptr; bkts = bkts->GetNextBuckets())
			bucketCount += bkts->GetCount();
		return bucketCount;
	}

	ConstBucketBounds GetBucketBounds(size_t bucketIndex) const
	{
		MOMO_CHECK(bucketIndex < GetBucketCount());
		size_t curBucketIndex = bucketIndex;
		for (Buckets* bkts = mBuckets; bkts != nullptr; bkts = bkts->GetNextBuckets())
		{
			BucketParams& bucketParams = bkts->GetBucketParams();
			size_t curBucketCount = bkts->GetCount();
			if (curBucketIndex < curBucketCount)
				return (*bkts)[curBucketIndex].GetBounds(bucketParams);
			curBucketIndex -= curBucketCount;
		}
		MOMO_ASSERT(false);
		return ConstBucketBounds();
	}

	size_t GetBucketIndex(const Key& key) const
	{
		MOMO_CHECK(mBuckets != nullptr);
		ConstPosition pos = pvFind(key);
		if (!!pos)
		{
			size_t bucketIndex = ConstPositionProxy::GetBucketIndex(pos);
			BucketIterator bucketIter = ConstPositionProxy::GetBucketIterator(pos);
			Buckets* buckets = pvFindBuckets(bucketIndex, bucketIter);
			for (Buckets* bkts = mBuckets; bkts != buckets; bkts = bkts->GetNextBuckets())
				bucketIndex += bkts->GetCount();
			return bucketIndex;
		}
		else
		{
			size_t hashCode = ConstPositionProxy::GetHashCode(pos);
			return Bucket::GetStartBucketIndex(hashCode, mBuckets->GetCount());	//?
		}
	}

	ConstPosition MakePosition(size_t hashCode) const noexcept
	{
		return ConstPositionProxy(hashCode, BucketIterator(), mCrew.GetVersion());
	}

	void CheckIterator(ConstIterator iter, bool allowEmpty = true) const
	{
		ConstPosition pos = iter;
		ConstPositionProxy::Check(pos, mCrew.GetVersion(), allowEmpty);
	}

private:
	void pvDestroy() noexcept
	{
		pvDestroy(mBuckets, true);
	}

	void pvDestroy(Buckets* buckets, bool destroyBucketParams) noexcept
	{
		if (buckets == nullptr)
			return;
		BucketParams& bucketParams = buckets->GetBucketParams();
		for (Bucket& bucket : *buckets)
			bucket.Clear(bucketParams);
		pvDestroy(buckets->ExtractNextBuckets(), false);
		buckets->Destroy(GetMemManager(), destroyBucketParams);
	}

	size_t pvGetNewLogBucketCount() const
	{
		const HashTraits& hashTraits = GetHashTraits();
		if (mBuckets == nullptr)
			return hashTraits.GetLogStartBucketCount();
		size_t logBucketCount = mBuckets->GetLogCount();
		size_t shift = hashTraits.GetBucketCountShift((size_t)1 << logBucketCount,
			bucketMaxItemCount);
		MOMO_CHECK(shift > 0);
		return logBucketCount + shift;
	}

	bool pvExtraCheck(ConstPosition pos) const noexcept
	{
		try
		{
			return pos == pvFind(ItemTraits::GetKey(*pos));
		}
		catch (...)
		{
			//?
			return false;
		}
	}

	template<typename KeyArg>
	ConstPosition pvFind(const KeyArg& key) const
	{
		size_t hashCode = GetHashTraits().GetHashCode(key);
		return pvFind(key, hashCode);
	}

	template<typename KeyArg>
	ConstPosition pvFind(const KeyArg& key, size_t hashCode) const
	{
		size_t indexCode = hashCode;
		BucketIterator bucketIter = pvFind(key, &indexCode);
		return ConstPositionProxy(indexCode, bucketIter, mCrew.GetVersion());
	}

	template<typename KeyArg>
	BucketIterator pvFind(const KeyArg& key, size_t* indexCode) const
	{
		size_t hashCode = *indexCode;
		const HashTraits& hashTraits = GetHashTraits();
		auto pred = [&key, &hashTraits] (const Item& item)
			{ return hashTraits.IsEqual(key, ItemTraits::GetKey(item)); };
		for (Buckets* bkts = mBuckets; bkts != nullptr; bkts = bkts->GetNextBuckets())
		{
			BucketParams& bucketParams = bkts->GetBucketParams();
			size_t bucketCount = bkts->GetCount();
			size_t bucketIndex = Bucket::GetStartBucketIndex(hashCode, bucketCount);
			Bucket* bucket = &(*bkts)[bucketIndex];
			size_t maxProbe = bucket->GetMaxProbe(bkts->GetLogCount());
			size_t probe = 0;
			while (true)
			{
				BucketIterator bucketIter = bucket->Find(bucketParams, pred, hashCode);
				if (bucketIter != BucketIterator())
				{
					*indexCode = bucketIndex;
					return bucketIter;
				}
				if (!bucket->WasFull() || probe >= maxProbe)
					break;
				++probe;
				bucketIndex = Bucket::GetNextBucketIndex(bucketIndex, hashCode,
					bucketCount, probe);
				bucket = &(*bkts)[bucketIndex];
			}
			if (areItemsNothrowRelocatable)
				break;
		}
		return BucketIterator();
	}

	template<bool extraCheck, typename ItemCreator>
	InsertResult pvInsert(const Key& key, ItemCreator&& itemCreator)
	{
		ConstPosition pos = pvFind(key);
		if (!!pos)
			return { pos, false };
		pos = pvAdd<extraCheck>(pos, std::forward<ItemCreator>(itemCreator));
		return { pos, true };
	}

	template<bool extraCheck, typename ItemCreator>
	ConstPosition pvAdd(ConstPosition pos, ItemCreator&& itemCreator)
	{
		ConstPositionProxy::Check(pos, mCrew.GetVersion(), false);
		MOMO_CHECK(ConstPositionProxy::GetBucketIterator(pos) == BucketIterator());
		size_t hashCode = ConstPositionProxy::GetHashCode(pos);
		ConstPosition resPos;
		if (mCount < mCapacity)
			resPos = pvAddNogrow<true>(*mBuckets, hashCode, std::forward<ItemCreator>(itemCreator));
		else
			resPos = pvAddGrow(hashCode, std::forward<ItemCreator>(itemCreator));
		if (mBuckets->GetNextBuckets() != nullptr)
		{
			BucketParams& bucketParams = mBuckets->GetBucketParams();
			size_t bucketIndex = ConstPositionProxy::GetBucketIndex(resPos);
			Bucket& bucket = (*mBuckets)[bucketIndex];
			size_t itemIndex = std::distance(bucket.GetBounds(bucketParams).GetBegin(),
				ConstPositionProxy::GetBucketIterator(resPos));
			pvRelocateItems();
			ConstPositionProxy::Reset(resPos, bucketIndex,
				std::next(bucket.GetBounds(bucketParams).GetBegin(), itemIndex));
		}
		MOMO_EXTRA_CHECK(!extraCheck || pvExtraCheck(resPos));
		return resPos;
	}

	template<bool incCount, typename ItemCreator>
	ConstPosition pvAddNogrow(Buckets& buckets, size_t hashCode, ItemCreator&& itemCreator)
	{
		size_t bucketCount = buckets.GetCount();
		size_t bucketIndex = Bucket::GetStartBucketIndex(hashCode, bucketCount);
		Bucket& startBucket = buckets[bucketIndex];
		Bucket* bucket = &startBucket;
		size_t probe = 0;
		while (bucket->IsFull())
		{
			++probe;
			if (probe >= bucketCount)
				throw std::runtime_error("momo::HashSet is full");
			bucketIndex = Bucket::GetNextBucketIndex(bucketIndex, hashCode, bucketCount, probe);
			bucket = &buckets[bucketIndex];
		}
		BucketIterator bucketIter = bucket->AddCrt(buckets.GetBucketParams(),
			std::forward<ItemCreator>(itemCreator), hashCode, buckets.GetLogCount(), probe);
		startBucket.UpdateMaxProbe(probe);
		if (incCount)
		{
			++mCount;
			mCrew.IncVersion();
		}
		return ConstPositionProxy(bucketIndex, bucketIter, mCrew.GetVersion());
	}

	template<typename ItemCreator>
	ConstPosition pvAddGrow(size_t hashCode, ItemCreator&& itemCreator)
	{
		const HashTraits& hashTraits = GetHashTraits();
		size_t newLogBucketCount = pvGetNewLogBucketCount();
		size_t newCapacity = hashTraits.CalcCapacity((size_t)1 << newLogBucketCount,
			bucketMaxItemCount);
		MOMO_CHECK(newCapacity > mCount);
		bool hasBuckets = (mBuckets != nullptr);
		Buckets* newBuckets;
		try
		{
			newBuckets = Buckets::Create(GetMemManager(), newLogBucketCount,
				hasBuckets ? &mBuckets->GetBucketParams() : nullptr);
		}
		catch (const std::bad_alloc& exception)
		{
			if (Settings::overloadIfCannotGrow && hasBuckets)
			{
				return pvAddNogrow<true>(*mBuckets, hashCode,
					std::forward<ItemCreator>(itemCreator));
			}
			throw exception;
		}
		ConstPosition resPos;
		try
		{
			resPos = pvAddNogrow<true>(*newBuckets, hashCode,
				std::forward<ItemCreator>(itemCreator));
		}
		catch (...)
		{
			newBuckets->Destroy(GetMemManager(), !hasBuckets);
			throw;
		}
		newBuckets->SetNextBuckets(mBuckets);
		mBuckets = newBuckets;
		mCapacity = newCapacity;
		return resPos;
	}

	template<typename ItemReplacer>
	ConstIterator pvRemove(ConstIterator iter, ItemReplacer itemReplacer)
	{
		MOMO_CHECK(mBuckets != nullptr);
		ConstPosition pos = iter;
		ConstPositionProxy::Check(pos, mCrew.GetVersion(), false);
		BucketIterator bucketIter = ConstPositionProxy::GetBucketIterator(pos);
		MOMO_CHECK(bucketIter != BucketIterator());
		size_t bucketIndex = ConstPositionProxy::GetBucketIndex(pos);
		Buckets* buckets = pvFindBuckets(bucketIndex, bucketIter);
		Bucket& bucket = (*buckets)[bucketIndex];
		bucketIter = bucket.Remove(buckets->GetBucketParams(), bucketIter, itemReplacer);
		--mCount;
		mCrew.IncVersion();
		if (!ConstIteratorProxy::IsMovable(iter))
			return ConstIterator();
		return ConstIteratorProxy(*buckets, bucketIndex, bucketIter, mCrew.GetVersion());
	}

	ConstIterator pvExtract(ConstIterator iter, Item* extItem)
	{
		auto itemReplacer = [this, extItem] (Item& srcItem, Item& dstItem)
		{
			MemManager& memManager = GetMemManager();
			if (std::addressof(srcItem) == std::addressof(dstItem))
				ItemTraits::Relocate(&memManager, srcItem, extItem);
			else
				ItemTraits::ReplaceRelocate(memManager, srcItem, dstItem, extItem);
		};
		return pvRemove(iter, itemReplacer);
	}

	Buckets* pvFindBuckets(size_t bucketIndex, BucketIterator bucketIter) const
	{
		MOMO_ASSERT(mBuckets != nullptr);
		if (mBuckets->GetNextBuckets() == nullptr)
			return mBuckets;
		for (Buckets* bkts = mBuckets; bkts != nullptr; bkts = bkts->GetNextBuckets())
		{
			if (bucketIndex >= bkts->GetCount())
				continue;
			Bucket& bucket = (*bkts)[bucketIndex];
			BucketBounds bucketBounds = bucket.GetBounds(bkts->GetBucketParams());
			std::less<BucketIterator> less;
			if (!less(bucketIter, bucketBounds.GetBegin()) && less(bucketIter, bucketBounds.GetEnd()))
				return bkts;
		}
		MOMO_ASSERT(false);
		return nullptr;
	}

	void pvRelocateItems() noexcept
	{
		Buckets* nextBuckets = mBuckets->GetNextBuckets();
		MOMO_ASSERT(nextBuckets != nullptr);
		try
		{
			pvRelocateItems(nextBuckets);
			mBuckets->ExtractNextBuckets();
		}
		catch (...)
		{
			// no throw!
		}
	}

	void pvRelocateItems(Buckets* buckets) noexcept(areItemsNothrowRelocatable)
	{
		Buckets* nextBuckets = buckets->GetNextBuckets();
		if (nextBuckets != nullptr)
		{
			pvRelocateItems(nextBuckets);
			buckets->ExtractNextBuckets();
		}
		MemManager& memManager = GetMemManager();
		const HashTraits& hashTraits = GetHashTraits();
		BucketParams& bucketParams = buckets->GetBucketParams();
		size_t bucketCount = buckets->GetCount();
		for (size_t i = 0; i < bucketCount; ++i)
		{
			Bucket& bucket = (*buckets)[i];
			BucketBounds bucketBounds = bucket.GetBounds(bucketParams);
			BucketIterator bucketIter = bucketBounds.GetEnd();
			auto hashCodeFullGetter = [&hashTraits, &bucketIter] ()
				{ return hashTraits.GetHashCode(ItemTraits::GetKey(*bucketIter)); };
			for (size_t c = bucketBounds.GetCount(); c > 0; --c)
			{
				--bucketIter;
				size_t hashCode = bucket.GetHashCodePart(hashCodeFullGetter, bucketIter, i,
					buckets->GetLogCount(), mBuckets->GetLogCount());
				auto itemReplacer = [this, hashCode, &memManager] (Item& backItem, Item& item)
				{
					(void)backItem;
					MOMO_ASSERT(std::addressof(backItem) == std::addressof(item));
					auto relocateCreator = [&memManager, &item] (Item* newItem)
						{ ItemTraits::Relocate(&memManager, item, newItem); };
					pvAddNogrow<false>(*mBuckets, hashCode, relocateCreator);
				};
				bucketIter = bucket.Remove(bucketParams, bucketIter, itemReplacer);
			}
		}
		buckets->Destroy(memManager, false);
	}

	template<typename Set>
	void pvMergeTo(Set& dstSet)
	{
		ConstIterator iter = GetBegin();
		while (!!iter)
		{
			auto itemCreator = [this, &iter] (Item* newItem)
				{ iter = pvExtract(iter, newItem); };
			if (!dstSet.InsertCrt(ItemTraits::GetKey(*iter), itemCreator).inserted)
				++iter;
		}
	}

private:
	Crew mCrew;
	size_t mCount;
	size_t mCapacity;
	Buckets* mBuckets;
};

template<typename TKey>
using HashSetOpen = HashSet<TKey, HashTraitsOpen<TKey>>;

namespace internal
{
	class NestedHashSetSettings : public HashSetSettings
	{
	public:
		static const CheckMode checkMode = CheckMode::assertion;
		static const ExtraCheckMode extraCheckMode = ExtraCheckMode::nothing;
		static const bool checkVersion = false;
	};
}

} // namespace momo

namespace std
{
	template<typename B, typename S>
	struct iterator_traits<momo::internal::HashSetConstIterator<B, S>>
		: public momo::internal::IteratorTraitsStd<momo::internal::HashSetConstIterator<B, S>,
			forward_iterator_tag>
	{
	};
} // namespace std

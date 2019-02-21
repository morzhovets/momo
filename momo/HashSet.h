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
					new(buckets + bucketIndex) Bucket();
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
			return reinterpret_cast<Bucket*>(this + 1);
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
				new(bucketParams) BucketParams(memManager);
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
			typename std::aligned_storage<std::alignment_of<Bucket>::value,
				std::alignment_of<Bucket>::value>::type mBucketPadding;
		};
	};

	template<typename TBuckets, typename TSettings>
	class HashSetConstIterator : private VersionKeeper<TSettings>
	{
	protected:
		typedef TBuckets Buckets;
		typedef TSettings Settings;
		typedef typename Buckets::Bucket Bucket;
		typedef typename Bucket::Item Item;

	public:
		typedef const Item& Reference;
		typedef const Item* Pointer;

		typedef HashSetConstIterator ConstIterator;

	private:
		typedef internal::VersionKeeper<Settings> VersionKeeper;

		typedef typename Bucket::Iterator BucketIterator;
		typedef typename Bucket::Bounds BucketBounds;

	public:
		explicit HashSetConstIterator() noexcept
			: mBuckets(nullptr),
			mHashCode(0),
			mBucketIterator()
		{
		}

		//operator ConstIterator() const noexcept

		HashSetConstIterator& operator++()
		{
			VersionKeeper::Check();
			MOMO_CHECK(mBucketIterator != BucketIterator());
			if (ptIsMovable())
			{
				++mBucketIterator;
				pvMoveIf();
			}
			else
			{
				*this = HashSetConstIterator();
			}
			return *this;
		}

		Pointer operator->() const
		{
			VersionKeeper::Check();
			MOMO_CHECK(mBucketIterator != BucketIterator());
			return std::addressof(*mBucketIterator);	//?
		}

		bool operator==(ConstIterator iter) const noexcept
		{
			return mBucketIterator == iter.mBucketIterator;
		}

		MOMO_MORE_HASH_ITERATOR_OPERATORS(HashSetConstIterator)

	protected:
		explicit HashSetConstIterator(Buckets& buckets, size_t bucketIndex,
			BucketIterator bucketIter, const size_t* version, bool movable) noexcept
			: VersionKeeper(version),
			mBuckets(&buckets),
			mBucketIndex(bucketIndex + (movable ? 0 : buckets.GetCount())),
			mBucketIterator(bucketIter)
		{
			if (movable)
				pvMoveIf();
		}

		explicit HashSetConstIterator(Buckets* buckets, size_t hashCode,
			const size_t* version) noexcept
			: VersionKeeper(version),
			mBuckets(buckets),
			mHashCode(hashCode),
			mBucketIterator()
		{
		}

		bool ptIsMovable() const noexcept
		{
			MOMO_ASSERT(mBucketIterator != BucketIterator() && mBuckets != nullptr);
			return mBucketIndex < mBuckets->GetCount();
		}

		size_t ptGetBucketIndex() const noexcept
		{
			MOMO_ASSERT(mBucketIterator != BucketIterator() && mBuckets != nullptr);
			size_t bucketCount = mBuckets->GetCount();
			return (mBucketIndex < bucketCount) ? mBucketIndex : mBucketIndex - bucketCount;
		}

		size_t ptGetHashCode() const noexcept
		{
			MOMO_ASSERT(mBucketIterator == BucketIterator());
			return mHashCode;
		}

		Buckets* ptGetBuckets() const noexcept
		{
			return mBuckets;
		}

		BucketIterator ptGetBucketIterator() const noexcept
		{
			return mBucketIterator;
		}

		void ptCheck(const size_t* version, bool empty) const
		{
			(void)empty;
			VersionKeeper::Check(version);
			MOMO_CHECK(empty || mBuckets != nullptr);
			MOMO_CHECK(empty != (mBucketIterator != BucketIterator()));
		}

	private:
		void pvMoveIf() noexcept
		{
			if (mBucketIterator == pvGetBucketBounds().GetEnd())
				pvMove();
		}

		void pvMove() noexcept
		{
			size_t bucketCount = mBuckets->GetCount();
			while (true)
			{
				++mBucketIndex;
				if (mBucketIndex >= bucketCount)
					break;
				BucketBounds bounds = pvGetBucketBounds();
				mBucketIterator = bounds.GetBegin();
				if (mBucketIterator != bounds.GetEnd())
					return;
			}
			Buckets* nextBuckets = mBuckets->GetNextBuckets();
			if (nextBuckets != nullptr)
			{
				mBuckets = nextBuckets;
				mBucketIndex = 0;
				mBucketIterator = pvGetBucketBounds().GetBegin();
				return pvMoveIf();	//?
			}
			*this = HashSetConstIterator();
		}

		BucketBounds pvGetBucketBounds() const noexcept
		{
			return (*mBuckets)[mBucketIndex].GetBounds(mBuckets->GetBucketParams());
		}

	private:
		Buckets* mBuckets;
		union
		{
			size_t mBucketIndex;
			size_t mHashCode;
		};
		BucketIterator mBucketIterator;
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
	typedef internal::HashSetConstIterator<Buckets, Settings> ConstIterator;
	typedef ConstIterator Iterator;	//?

	typedef internal::InsertResult<ConstIterator> InsertResult;

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

	struct ItemPosition
	{
		size_t bucketIndex;
		BucketIterator bucketIterator;
	};

	struct ConstIteratorProxy : public ConstIterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, IsMovable, bool)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetBucketIndex, size_t)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetHashCode, size_t)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetBuckets, Buckets*)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetBucketIterator, BucketIterator)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, Check, void)
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
				pvAddNogrow(*mBuckets, hashCode, Creator<const Item&>(GetMemManager(), item));
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
		return pvMakeIterator(*mBuckets, 0,
			mBuckets->GetBegin()->GetBounds(mBuckets->GetBucketParams()).GetBegin(), true);
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

	ConstIterator Find(const Key& key) const
	{
		return pvFind(key);
	}

	template<typename KeyArg>
	internal::EnableIf<IsValidKeyArg<KeyArg>::value, ConstIterator> Find(const KeyArg& key) const
	{
		return pvFind(key);
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
	ConstIterator AddCrt(ConstIterator iter, ItemCreator&& itemCreator)
	{
		return pvAdd<extraCheck>(iter, std::forward<ItemCreator>(itemCreator));
	}

	template<typename... ItemArgs>
	ConstIterator AddVar(ConstIterator iter, ItemArgs&&... itemArgs)
	{
		return AddCrt(iter,
			Creator<ItemArgs...>(GetMemManager(), std::forward<ItemArgs>(itemArgs)...));
	}

	ConstIterator Add(ConstIterator iter, Item&& item)
	{
		return AddVar(iter, std::move(item));
	}

	ConstIterator Add(ConstIterator iter, const Item& item)
	{
		return AddVar(iter, item);
	}

	ConstIterator Add(ConstIterator iter, ExtractedItem&& extItem)
	{
		MOMO_CHECK(!extItem.IsEmpty());
		MemManager& memManager = GetMemManager();
		auto itemCreator = [&memManager, &extItem] (Item* newItem)
		{
			auto itemRemover = [&memManager, newItem] (Item& item)
				{ ItemTraits::Relocate(&memManager, item, newItem); };
			extItem.Remove(itemRemover);
		};
		return AddCrt(iter, itemCreator);
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
		ConstIterator iter = Find(key);
		if (!iter)
			return false;
		Remove(iter);
		return true;
	}

	ExtractedItem Extract(ConstIterator iter)
	{
		return ExtractedItem(*this, iter);	// need RVO for exception safety
	}

	template<typename KeyArg, bool extraCheck = true>
	void ResetKey(ConstIterator iter, KeyArg&& keyArg)
	{
		ConstIteratorProxy::Check(iter, mCrew.GetVersion(), false);
		MOMO_CHECK(ConstIteratorProxy::GetBuckets(iter) != nullptr);
		Item& item = *ConstIteratorProxy::GetBucketIterator(iter);
		ItemTraits::AssignKey(GetMemManager(), std::forward<KeyArg>(keyArg), item);
		MOMO_EXTRA_CHECK(!extraCheck || pvExtraCheck(iter));
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
		ConstIterator iter = Find(key);
		Buckets* buckets = ConstIteratorProxy::GetBuckets(iter);
		if (!!iter)
		{
			size_t bucketIndex = ConstIteratorProxy::GetBucketIndex(iter);
			for (Buckets* bkts = mBuckets; bkts != buckets; bkts = bkts->GetNextBuckets())
				bucketIndex += bkts->GetCount();
			return bucketIndex;
		}
		else
		{
			MOMO_ASSERT(buckets == mBuckets);
			size_t hashCode = ConstIteratorProxy::GetHashCode(iter);
			return Bucket::GetStartBucketIndex(hashCode, mBuckets->GetCount());	//?
		}
	}

	ConstIterator MakeIterator(size_t hashCode) const noexcept
	{
		return ConstIteratorProxy(mBuckets, hashCode, mCrew.GetVersion());
	}

	void CheckIterator(ConstIterator iter) const
	{
		if (ConstIteratorProxy::GetBuckets(iter) == nullptr)	//?
			return;
		ConstIteratorProxy::Check(iter, mCrew.GetVersion(), !iter);
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

	ConstIterator pvMakeIterator(Buckets& buckets, size_t bucketIndex, BucketIterator bucketIter,
		bool movable) const noexcept
	{
		return ConstIteratorProxy(buckets, bucketIndex, bucketIter, mCrew.GetVersion(), movable);
	}

	bool pvExtraCheck(ConstIterator iter) const noexcept
	{
		try
		{
			return iter == Find(ItemTraits::GetKey(*iter));
		}
		catch (...)
		{
			//?
			return false;
		}
	}

	template<typename KeyArg>
	ConstIterator pvFind(const KeyArg& key) const
	{
		const HashTraits& hashTraits = GetHashTraits();
		size_t hashCode = hashTraits.GetHashCode(key);
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
					return pvMakeIterator(*bkts, bucketIndex, bucketIter, false);
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
		return MakeIterator(hashCode);
	}

	template<bool extraCheck, typename ItemCreator>
	InsertResult pvInsert(const Key& key, ItemCreator&& itemCreator)
	{
		ConstIterator iter = Find(key);
		if (!!iter)
			return { iter, false };
		iter = pvAdd<extraCheck>(iter, std::forward<ItemCreator>(itemCreator));
		return { iter, true };
	}

	template<bool extraCheck, typename ItemCreator>
	ConstIterator pvAdd(ConstIterator iter, ItemCreator&& itemCreator)
	{
		ConstIteratorProxy::Check(iter, mCrew.GetVersion(), true);
		MOMO_CHECK(ConstIteratorProxy::GetBuckets(iter) == mBuckets);
		size_t hashCode = ConstIteratorProxy::GetHashCode(iter);
		ItemPosition itemPos;
		if (mCount < mCapacity)
			itemPos = pvAddNogrow(*mBuckets, hashCode, std::forward<ItemCreator>(itemCreator));
		else
			itemPos = pvAddGrow(hashCode, std::forward<ItemCreator>(itemCreator));
		if (mBuckets->GetNextBuckets() != nullptr)
		{
			BucketParams& bucketParams = mBuckets->GetBucketParams();
			Bucket& bucket = (*mBuckets)[itemPos.bucketIndex];
			size_t itemIndex = std::distance(bucket.GetBounds(bucketParams).GetBegin(),
				itemPos.bucketIterator);
			pvRelocateItems();
			itemPos.bucketIterator = std::next(bucket.GetBounds(bucketParams).GetBegin(),
				itemIndex);	//?
		}
		++mCount;
		mCrew.IncVersion();
		ConstIterator resIter = pvMakeIterator(*mBuckets, itemPos.bucketIndex,
			itemPos.bucketIterator, false);
		MOMO_EXTRA_CHECK(!extraCheck || pvExtraCheck(resIter));
		return resIter;
	}

	template<typename ItemCreator>
	ItemPosition pvAddNogrow(Buckets& buckets, size_t hashCode, ItemCreator&& itemCreator)
	{
		size_t bucketCount = buckets.GetCount();
		size_t bucketIndex = Bucket::GetStartBucketIndex(hashCode, bucketCount);
		Bucket* bucket = &buckets[bucketIndex];
		size_t probe = 0;
		while (bucket->IsFull())
		{
			++probe;
			if (probe >= bucketCount)
				throw std::runtime_error("momo::HashSet is full");
			bucketIndex = Bucket::GetNextBucketIndex(bucketIndex, hashCode, bucketCount, probe);
			bucket = &buckets[bucketIndex];
		}
		ItemPosition itemPos;
		itemPos.bucketIndex = bucketIndex;
		itemPos.bucketIterator = bucket->AddCrt(buckets.GetBucketParams(),
			std::forward<ItemCreator>(itemCreator), hashCode, buckets.GetLogCount(), probe);
		return itemPos;
	}

	template<typename ItemCreator>
	ItemPosition pvAddGrow(size_t hashCode, ItemCreator&& itemCreator)
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
				return pvAddNogrow(*mBuckets, hashCode, std::forward<ItemCreator>(itemCreator));
			throw exception;
		}
		ItemPosition itemPos;
		try
		{
			itemPos = pvAddNogrow(*newBuckets, hashCode, std::forward<ItemCreator>(itemCreator));
		}
		catch (...)
		{
			newBuckets->Destroy(GetMemManager(), !hasBuckets);
			throw;
		}
		newBuckets->SetNextBuckets(mBuckets);
		mBuckets = newBuckets;
		mCapacity = newCapacity;
		return itemPos;
	}

	template<typename ItemReplacer>
	ConstIterator pvRemove(ConstIterator iter, ItemReplacer itemReplacer)
	{
		ConstIteratorProxy::Check(iter, mCrew.GetVersion(), false);
		Buckets* buckets = ConstIteratorProxy::GetBuckets(iter);
		MOMO_CHECK(buckets != nullptr);
		BucketParams& bucketParams = buckets->GetBucketParams();
		size_t bucketIndex = ConstIteratorProxy::GetBucketIndex(iter);
		Bucket& bucket = (*buckets)[bucketIndex];
		BucketIterator bucketIter = ConstIteratorProxy::GetBucketIterator(iter);
		bucketIter = bucket.Remove(bucketParams, bucketIter, itemReplacer);
		--mCount;
		mCrew.IncVersion();
		if (!ConstIteratorProxy::IsMovable(iter))
			return ConstIterator();
		return pvMakeIterator(*buckets, bucketIndex, bucketIter, true);
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
					pvAddNogrow(*mBuckets, hashCode, relocateCreator);
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

} // namespace momo

namespace std
{
	template<typename B, typename S>
	struct iterator_traits<momo::internal::HashSetConstIterator<B, S>>
	{
		typedef forward_iterator_tag iterator_category;
		typedef ptrdiff_t difference_type;
		typedef typename momo::internal::HashSetConstIterator<B, S>::Pointer pointer;
		typedef typename momo::internal::HashSetConstIterator<B, S>::Reference reference;
		typedef typename std::decay<reference>::type value_type;
	};
} // namespace std

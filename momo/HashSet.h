/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/HashSet.h

  namespace momo:
    class HashSetItemTraits
    struct HashSetSettings
    class HashSet

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
#include "Array.h"

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
		typedef typename Bucket::ConstBounds ConstBucketBounds;

		static const size_t maxBucketCount =
			(SIZE_MAX - sizeof(size_t) - 2 * sizeof(void*)) / sizeof(Bucket);

	public:
		HashSetBuckets() = delete;

		HashSetBuckets(const HashSetBuckets&) = delete;

		~HashSetBuckets() = delete;

		HashSetBuckets& operator=(const HashSetBuckets&) = delete;

		static HashSetBuckets& Create(MemManager& memManager, size_t bucketCount,
			BucketParams* bucketParams)
		{
			if (bucketCount > maxBucketCount)
				throw std::length_error("momo::internal::HashSetBuckets length error");
			size_t bufferSize = pvGetBufferSize(bucketCount);
			HashSetBuckets& resBuckets = *memManager.template Allocate<HashSetBuckets>(bufferSize);
			resBuckets.mCount = 0;
			resBuckets.mNextBuckets = nullptr;
			Bucket* buckets = resBuckets.pvGetBuckets();
			try
			{
				size_t& curBucketCount = resBuckets.mCount;
				for (; curBucketCount < bucketCount; ++curBucketCount)
					new(buckets + curBucketCount) Bucket();
				if (bucketParams == nullptr)
					resBuckets.mBucketParams = &pvCreateBucketParams(memManager);
				else
					resBuckets.mBucketParams = bucketParams;
			}
			catch (...)
			{
				for (size_t i = 0; i < resBuckets.mCount; ++i)
					buckets[i].~Bucket();
				memManager.Deallocate(&resBuckets, bufferSize);
				throw;
			}
			return resBuckets;
		}

		void Destroy(MemManager& memManager, bool destroyBucketParams) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mNextBuckets == nullptr);
			size_t bucketCount = GetCount();
			Bucket* buckets = pvGetBuckets();
			for (size_t i = 0; i < bucketCount; ++i)
				buckets[i].~Bucket();
			if (destroyBucketParams)
			{
				mBucketParams->~BucketParams();
				memManager.Deallocate(mBucketParams, sizeof(BucketParams));
			}
			memManager.Deallocate(this, pvGetBufferSize(bucketCount));
		}

		Bucket* GetBegin() MOMO_NOEXCEPT
		{
			return pvGetBuckets();
		}

		Bucket* GetEnd() MOMO_NOEXCEPT
		{
			return pvGetBuckets() + GetCount();
		}

		MOMO_FRIENDS_BEGIN_END(HashSetBuckets&, Bucket*)

		const HashSetBuckets* GetNextBuckets() const MOMO_NOEXCEPT
		{
			return mNextBuckets;
		}

		HashSetBuckets* GetNextBuckets() MOMO_NOEXCEPT
		{
			return mNextBuckets;
		}

		HashSetBuckets* ExtractNextBuckets() MOMO_NOEXCEPT
		{
			HashSetBuckets* nextBuckets = mNextBuckets;
			mNextBuckets = nullptr;
			return nextBuckets;
		}

		void SetNextBuckets(HashSetBuckets* nextBuckets) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mNextBuckets == nullptr);
			mNextBuckets = nextBuckets;
		}

		size_t GetCount() const MOMO_NOEXCEPT
		{
			return mCount;
		}

		const Bucket& operator[](size_t index) const MOMO_NOEXCEPT
		{
			return pvGetBuckets()[index];
		}

		Bucket& operator[](size_t index) MOMO_NOEXCEPT
		{
			return pvGetBuckets()[index];
		}

		ConstBucketBounds GetBucketBounds(size_t index) const MOMO_NOEXCEPT
		{
			const BucketParams& bucketParams = *mBucketParams;
			return pvGetBuckets()[index].GetBounds(bucketParams);
		}

		BucketParams& GetBucketParams() MOMO_NOEXCEPT
		{
			return *mBucketParams;
		}

	private:
		const Bucket* pvGetBuckets() const MOMO_NOEXCEPT
		{
			return reinterpret_cast<const Bucket*>(this + 1);
		}

		Bucket* pvGetBuckets() MOMO_NOEXCEPT
		{
			return reinterpret_cast<Bucket*>(this + 1);
		}

		static size_t pvGetBufferSize(size_t bucketCount) MOMO_NOEXCEPT
		{
			return sizeof(HashSetBuckets) + bucketCount * sizeof(Bucket);
		}

		static BucketParams& pvCreateBucketParams(MemManager& memManager)
		{
			BucketParams* bucketParams = memManager.template Allocate<BucketParams>(
				sizeof(BucketParams));
			try
			{
				new(bucketParams) BucketParams(memManager);
			}
			catch (...)
			{
				memManager.Deallocate(bucketParams, sizeof(BucketParams));
				throw;
			}
			return *bucketParams;
		}

	private:
		size_t mCount;
		HashSetBuckets* mNextBuckets;
		union
		{
			BucketParams* mBucketParams;
			typename std::aligned_storage<std::alignment_of<Bucket>::value,
				std::alignment_of<Bucket>::value>::type mBucketPadding;
		};
	};

	template<typename TBuckets, typename TSettings>
	class HashSetConstIterator : private IteratorVersion<TSettings::checkVersion>
	{
	protected:
		typedef TBuckets Buckets;
		typedef TSettings Settings;
		typedef typename Buckets::Bucket Bucket;
		typedef typename Bucket::Item Item;
		typedef typename Bucket::ConstBounds ConstBucketBounds;

	public:
		typedef const Item& Reference;
		typedef const Item* Pointer;

		typedef HashSetConstIterator ConstIterator;

	private:
		typedef internal::IteratorVersion<Settings::checkVersion> IteratorVersion;

	public:
		HashSetConstIterator() MOMO_NOEXCEPT
			: mBuckets(nullptr),
			mHashCode(0),
			mItemPtr(nullptr)
		{
		}

		//operator ConstIterator() const MOMO_NOEXCEPT

		HashSetConstIterator& operator++()
		{
			MOMO_CHECK(mItemPtr != nullptr);
			MOMO_CHECK(IteratorVersion::Check());
			if (ptIsMovable())
			{
				++mItemPtr;
				pvMove();
			}
			else
			{
				*this = HashSetConstIterator();
			}
			return *this;
		}

		Pointer operator->() const
		{
			MOMO_CHECK(mItemPtr != nullptr);
			MOMO_CHECK(IteratorVersion::Check());
			return mItemPtr;
		}

		bool operator==(ConstIterator iter) const MOMO_NOEXCEPT
		{
			return mItemPtr == iter.mItemPtr;
		}

		MOMO_MORE_HASH_ITERATOR_OPERATORS(HashSetConstIterator)

	protected:
		HashSetConstIterator(const Buckets& buckets, size_t bucketIndex, const Item* pitem,
			const size_t* version, bool movable) MOMO_NOEXCEPT
			: IteratorVersion(version),
			mBuckets(&buckets),
			mBucketIndex(bucketIndex + (movable ? 0 : buckets.GetCount())),
			mItemPtr(pitem)
		{
			if (movable)
				pvMove();
		}

		HashSetConstIterator(const Buckets* buckets, size_t hashCode,
			const size_t* version) MOMO_NOEXCEPT
			: IteratorVersion(version),
			mBuckets(buckets),
			mHashCode(hashCode),
			mItemPtr(nullptr)
		{
		}

		bool ptIsMovable() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mItemPtr != nullptr && mBuckets != nullptr);
			return mBucketIndex < mBuckets->GetCount();
		}

		size_t ptGetBucketIndex() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mItemPtr != nullptr && mBuckets != nullptr);
			size_t bucketCount = mBuckets->GetCount();
			return (mBucketIndex < bucketCount) ? mBucketIndex : mBucketIndex - bucketCount;
		}

		size_t ptGetHashCode() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mItemPtr == nullptr);
			return mHashCode;
		}

		const Buckets* ptGetBuckets() const MOMO_NOEXCEPT
		{
			return mBuckets;
		}

		void ptCheck(const size_t* version, bool empty) const
		{
			(void)version;
			(void)empty;
			MOMO_CHECK(empty || mBuckets != nullptr);
			MOMO_CHECK(empty ^ (mItemPtr != nullptr));
			MOMO_CHECK(IteratorVersion::Check(version));
		}

	private:
		void pvMove() MOMO_NOEXCEPT
		{
			if (mItemPtr != pvGetBucketBounds().GetEnd())
				return;
			size_t bucketCount = mBuckets->GetCount();
			while (true)
			{
				++mBucketIndex;
				if (mBucketIndex >= bucketCount)
					break;
				ConstBucketBounds bounds = pvGetBucketBounds();
				mItemPtr = bounds.GetBegin();
				if (mItemPtr != bounds.GetEnd())
					return;
			}
			const Buckets* nextBuckets = mBuckets->GetNextBuckets();
			if (nextBuckets != nullptr)
			{
				mBuckets = nextBuckets;
				mBucketIndex = 0;
				mItemPtr = pvGetBucketBounds().GetBegin();
				return pvMove();	//?
			}
			*this = HashSetConstIterator();
		}

		ConstBucketBounds pvGetBucketBounds() const MOMO_NOEXCEPT
		{
			return mBuckets->GetBucketBounds(mBucketIndex);
		}

	private:
		const Buckets* mBuckets;
		union
		{
			size_t mBucketIndex;
			size_t mHashCode;
		};
		const Item* mItemPtr;
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
		static void Destroy(MemManager& memManager, Item* items, size_t count) MOMO_NOEXCEPT
		{
			for (size_t i = 0; i < count; ++i)
				HashSetItemTraits::Destroy(&memManager, items[i]);
		}

		template<typename ItemCreator>
		static void RelocateCreate(MemManager& memManager, Item* srcItems, Item* dstItems,
			size_t count, const ItemCreator& itemCreator, Item* newItem)
		{
			HashSetItemTraits::RelocateCreate(memManager, srcItems, dstItems, count,
				itemCreator, newItem);
		}
	};
}

template<typename TKey, typename TItem, typename TMemManager>
struct HashSetItemTraits : public internal::SetItemTraits<TKey, TItem, TMemManager>
{
public:
	typedef TKey Key;
	typedef TItem Item;
	typedef TMemManager MemManager;

private:
	typedef internal::ObjectManager<Item, MemManager> ItemManager;

public:
	template<typename ItemCreator>
	static void RelocateCreate(MemManager& memManager, Item* srcItems, Item* dstItems,
		size_t count, const ItemCreator& itemCreator, Item* newItem)
	{
		ItemManager::RelocateCreate(memManager, srcItems, dstItems, count, itemCreator, newItem);
	}
};

struct HashSetSettings
{
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

	typedef typename HashTraits::HashBucket HashBucket;
	typedef typename HashBucket::template Bucket<BucketItemTraits> Bucket;

	typedef typename Bucket::Params BucketParams;

	typedef internal::HashSetBuckets<Bucket> Buckets;

	typedef internal::ArrayPtrIterator<const Item, Settings> ConstBucketIterator;

	template<typename... ItemArgs>
	using Creator = typename ItemTraits::template Creator<ItemArgs...>;

public:
	typedef internal::HashSetConstIterator<Buckets, Settings> ConstIterator;
	typedef ConstIterator Iterator;

	typedef internal::InsertResult<ConstIterator> InsertResult;

	typedef internal::SetExtractedItem<ItemTraits, Settings> ExtractedItem;

	typedef internal::HashDerivedBucketBounds<ConstBucketIterator,
		typename Bucket::ConstBounds> ConstBucketBounds;

private:
	struct ConstIteratorProxy : public ConstIterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, IsMovable, bool)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetBucketIndex, size_t)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetHashCode, size_t)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetBuckets, const Buckets*)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, Check, void)
	};

public:
	explicit HashSet(const HashTraits& hashTraits = HashTraits(),
		MemManager&& memManager = MemManager())
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

	HashSet(HashSet&& hashSet) MOMO_NOEXCEPT
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
		size_t bucketCount = (size_t)1 << hashTraits.GetLogStartBucketCount();
		while (true)
		{
			mCapacity = hashTraits.CalcCapacity(bucketCount);
			if (mCapacity >= mCount)
				break;
			bucketCount <<= 1;
		}
		mBuckets = &Buckets::Create(GetMemManager(), bucketCount, nullptr);
		BucketParams& bucketParams = mBuckets->GetBucketParams();
		try
		{
			for (const Item& item : hashSet)
			{
				size_t hashCode = hashTraits.GetHashCode(ItemTraits::GetKey(item));
				size_t bucketIndex = pvGetBucketIndexForAdd(*mBuckets, hashCode);
				(*mBuckets)[bucketIndex].AddBackCrt(bucketParams,
					Creator<const Item&>(GetMemManager(), item));
			}
		}
		catch (...)
		{
			pvDestroy();
			throw;
		}
	}

	~HashSet() MOMO_NOEXCEPT
	{
		pvDestroy();
	}

	HashSet& operator=(HashSet&& hashSet) MOMO_NOEXCEPT
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

	void Swap(HashSet& hashSet) MOMO_NOEXCEPT
	{
		mCrew.Swap(hashSet.mCrew);
		std::swap(mCount, hashSet.mCount);
		std::swap(mCapacity, hashSet.mCapacity);
		std::swap(mBuckets, hashSet.mBuckets);
	}

	ConstIterator GetBegin() const MOMO_NOEXCEPT
	{
		if (mCount == 0)
			return ConstIterator();
		return pvMakeIterator(*mBuckets, 0, mBuckets->GetBucketBounds(0).GetBegin(), true);
	}

	ConstIterator GetEnd() const MOMO_NOEXCEPT
	{
		return ConstIterator();
	}

	MOMO_FRIEND_SWAP(HashSet)
	MOMO_FRIENDS_BEGIN_END(const HashSet&, ConstIterator)

	const HashTraits& GetHashTraits() const MOMO_NOEXCEPT
	{
		return mCrew.GetContainerTraits();
	}

	const MemManager& GetMemManager() const MOMO_NOEXCEPT
	{
		return mCrew.GetMemManager();
	}

	MemManager& GetMemManager() MOMO_NOEXCEPT
	{
		return mCrew.GetMemManager();
	}

	size_t GetCount() const MOMO_NOEXCEPT
	{
		return mCount;
	}

	bool IsEmpty() const MOMO_NOEXCEPT
	{
		return mCount == 0;
	}

	void Clear(bool shrink = true) MOMO_NOEXCEPT
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

	size_t GetCapacity() const MOMO_NOEXCEPT
	{
		return mCapacity;
	}

	void Reserve(size_t capacity)
	{
		if (capacity <= mCapacity)
			return;
		const HashTraits& hashTraits = GetHashTraits();
		size_t newBucketCount = pvGetNewBucketCount();
		size_t newCapacity;
		while (true)
		{
			newCapacity = hashTraits.CalcCapacity(newBucketCount);
			if (newCapacity >= capacity)
				break;
			newBucketCount <<= 1;
		}
		Buckets& newBuckets = Buckets::Create(GetMemManager(), newBucketCount,
			(mBuckets != nullptr) ? &mBuckets->GetBucketParams() : nullptr);
		newBuckets.SetNextBuckets(mBuckets);
		mBuckets = &newBuckets;
		mCapacity = newCapacity;
		mCrew.IncVersion();
		if (mBuckets->GetNextBuckets() != nullptr)
			pvMoveItems();
	}

	void Shrink()
	{
		HashSet(*this).Swap(*this);
	}

	ConstIterator Find(const Key& key) const
	{
		return pvFind(key);
	}

	template<typename KeyArg,
		bool isValidKeyArg = HashTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, ConstIterator>::type Find(const KeyArg& key) const
	{
		return pvFind(key);
	}

	bool HasKey(const Key& key) const
	{
		return !!pvFind(key);
	}

	template<typename KeyArg,
		bool isValidKeyArg = HashTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, bool>::type HasKey(const KeyArg& key) const
	{
		return !!pvFind(key);
	}

	template<typename ItemCreator>
	InsertResult InsertCrt(const Key& key, const ItemCreator& itemCreator)
	{
		return pvInsert<true>(key, itemCreator);
	}

	template<typename... ItemArgs>
	InsertResult InsertVar(const Key& key, ItemArgs&&... itemArgs)
	{
		return InsertCrt(key,
			Creator<ItemArgs...>(GetMemManager(), std::forward<ItemArgs>(itemArgs)...));
	}

	InsertResult Insert(Item&& item)
	{
		return pvInsert<false>(ItemTraits::GetKey(static_cast<const Item&>(item)),
			Creator<Item>(GetMemManager(), std::move(item)));
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
				{ ItemTraits::Relocate(memManager, item, newItem); };
			extItem.Reset(itemRemover);
		};
		return pvInsert<false>(ItemTraits::GetKey(extItem.GetItem()), itemCreator);
	}

	template<typename ArgIterator>
	size_t Insert(ArgIterator begin, ArgIterator end)
	{
		MOMO_CHECK_TYPE(Item, *begin);
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
	ConstIterator AddCrt(ConstIterator iter, const ItemCreator& itemCreator)
	{
		return pvAdd<extraCheck>(iter, itemCreator);
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
				{ ItemTraits::Relocate(memManager, item, newItem); };
			extItem.Reset(itemRemover);
		};
		return AddCrt(iter, itemCreator);
	}

	ConstIterator Remove(ConstIterator iter)
	{
		auto replaceFunc = [this] (Item& srcItem, Item& dstItem)
			{ ItemTraits::Replace(GetMemManager(), srcItem, dstItem); };
		return pvRemove(iter, replaceFunc);
	}

	ConstIterator Remove(ConstIterator iter, ExtractedItem& extItem)
	{
		MOMO_CHECK(extItem.IsEmpty());
		MemManager& memManager = GetMemManager();
		auto replaceFunc = [&memManager, &extItem] (Item& srcItem, Item& dstItem)
		{
			auto itemCreator = [&memManager, &srcItem, &dstItem] (Item* newItem)
			{
				if (std::addressof(srcItem) == std::addressof(dstItem))
					ItemTraits::Relocate(memManager, srcItem, newItem);
				else
					ItemTraits::ReplaceRelocate(memManager, srcItem, dstItem, newItem);
			};
			extItem.Set(memManager, itemCreator);
		};
		return pvRemove(iter, replaceFunc);
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

	void ResetKey(ConstIterator iter, Key&& newKey)
	{
		Item& item = pvGetItemForReset(iter, static_cast<const Key&>(newKey));
		ItemTraits::AssignKey(GetMemManager(), std::move(newKey), item);
	}

	void ResetKey(ConstIterator iter, const Key& newKey)
	{
		Item& item = pvGetItemForReset(iter, newKey);
		ItemTraits::AssignKey(GetMemManager(), newKey, item);
	}

	template<typename Set>
	void MergeFrom(Set& srcSet)
	{
		srcSet.MergeTo(*this);
	}

	template<typename Set>
	void MergeTo(Set& dstSet)
	{
		MOMO_STATIC_ASSERT((std::is_same<ItemTraits, typename Set::ItemTraits>::value));
		ConstIterator iter = GetBegin();
		while (!!iter)
		{
			auto itemCreator = [this, &iter] (Item* newItem)
			{
				MemManager& memManager = GetMemManager();
				auto replaceFunc = [&memManager, newItem] (Item& srcItem, Item& dstItem)
				{
					if (std::addressof(srcItem) == std::addressof(dstItem))
						ItemTraits::Relocate(memManager, srcItem, newItem);
					else
						ItemTraits::ReplaceRelocate(memManager, srcItem, dstItem, newItem);
				};
				iter = pvRemove(iter, replaceFunc);
			};
			if (!dstSet.InsertCrt(ItemTraits::GetKey(*iter), itemCreator).inserted)
				++iter;
		}
	}

	size_t GetBucketCount() const MOMO_NOEXCEPT
	{
		size_t bucketCount = 0;
		for (const Buckets* bkts = mBuckets; bkts != nullptr; bkts = bkts->GetNextBuckets())
			bucketCount += bkts->GetCount();
		return bucketCount;
	}

	ConstBucketBounds GetBucketBounds(size_t bucketIndex) const
	{
		MOMO_CHECK(bucketIndex < GetBucketCount());
		size_t curBucketIndex = bucketIndex;
		for (const Buckets* bkts = mBuckets; bkts != nullptr; bkts = bkts->GetNextBuckets())
		{
			size_t curBucketCount = bkts->GetCount();
			if (curBucketIndex < curBucketCount)
				return ConstBucketBounds(bkts->GetBucketBounds(curBucketIndex));
			curBucketIndex -= curBucketCount;
		}
		MOMO_ASSERT(false);
		return ConstBucketBounds();
	}

	size_t GetBucketIndex(const Key& key) const
	{
		MOMO_CHECK(mBuckets != nullptr);
		ConstIterator iter = Find(key);
		if (!iter)
			return pvGetBucketIndexForAdd(*mBuckets, ConstIteratorProxy::GetHashCode(iter));	//?
		const Buckets* buckets = ConstIteratorProxy::GetBuckets(iter);
		size_t bucketIndex = ConstIteratorProxy::GetBucketIndex(iter);
		for (const Buckets* bkts = mBuckets; bkts != buckets; bkts = bkts->GetNextBuckets())
			bucketIndex += bkts->GetCount();
		return bucketIndex;
	}

private:
	void pvDestroy() MOMO_NOEXCEPT
	{
		pvDestroy(mBuckets, true);
	}

	void pvDestroy(Buckets* buckets, bool destroyBucketParams) MOMO_NOEXCEPT
	{
		if (buckets == nullptr)
			return;
		BucketParams& bucketParams = buckets->GetBucketParams();
		for (Bucket& bucket : *buckets)
			bucket.Clear(bucketParams);
		pvDestroy(buckets->ExtractNextBuckets(), false);
		buckets->Destroy(GetMemManager(), destroyBucketParams);
	}

	size_t pvGetNewBucketCount() const
	{
		const HashTraits& hashTraits = GetHashTraits();
		if (mBuckets == nullptr)
			return (size_t)1 << hashTraits.GetLogStartBucketCount();
		size_t bucketCount = mBuckets->GetCount();
		size_t shift = hashTraits.GetBucketCountShift(bucketCount);
		MOMO_CHECK(shift > 0);
		return bucketCount << shift;
	}

	ConstIterator pvMakeIterator(const Buckets& buckets, size_t bucketIndex,
		const Item* pitem, bool movable) const MOMO_NOEXCEPT
	{
		return ConstIteratorProxy(buckets, bucketIndex, pitem, mCrew.GetVersion(), movable);
	}

	bool pvExtraCheck(ConstIterator iter) const MOMO_NOEXCEPT
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
		for (const Buckets* bkts = mBuckets; bkts != nullptr; bkts = bkts->GetNextBuckets())
		{
			const BucketParams& bucketParams = mBuckets->GetBucketParams();
			size_t bucketCount = bkts->GetCount();
			for (size_t probe = 0; probe < bucketCount; ++probe)
			{
				size_t bucketIndex = pvGetBucketIndex(hashCode, bucketCount, probe);
				const Bucket& bucket = (*bkts)[bucketIndex];
				for (const Item& item : bucket.GetBounds(bucketParams))
				{
					if (hashTraits.IsEqual(key, ItemTraits::GetKey(item)))
						return pvMakeIterator(*bkts, bucketIndex, std::addressof(item), false);
				}
				if (!bucket.WasFull())
					break;
			}
		}
		return ConstIteratorProxy(mBuckets, hashCode, mCrew.GetVersion());
	}

	size_t pvGetBucketIndex(size_t hashCode, size_t bucketCount, size_t probe) const
	{
		const HashTraits& hashTraits = GetHashTraits();
		size_t bucketIndex = hashTraits.GetBucketIndex(hashCode, bucketCount, probe);
		MOMO_ASSERT(bucketIndex < bucketCount);
		return bucketIndex;
	}

	size_t pvGetBucketIndexForAdd(const Buckets& buckets, size_t hashCode) const
	{
		size_t bucketCount = buckets.GetCount();
		for (size_t probe = 0; probe < bucketCount; ++probe)
		{
			size_t bucketIndex = pvGetBucketIndex(hashCode, bucketCount, probe);
			if (!buckets[bucketIndex].IsFull())
				return bucketIndex;
		}
		throw std::runtime_error("momo::HashSet is full");
	}

	Buckets* pvGetMutBuckets(ConstIterator iter) MOMO_NOEXCEPT
	{
		const Buckets* buckets = ConstIteratorProxy::GetBuckets(iter);
		for (Buckets* bkts = mBuckets; bkts != nullptr; bkts = bkts->GetNextBuckets())
		{
			if (bkts == buckets)
				return bkts;
		}
		return nullptr;
	}

	Item& pvGetItemForReset(ConstIterator iter, const Key& newKey)
	{
		ConstIteratorProxy::Check(iter, mCrew.GetVersion(), false);
		Buckets* buckets = pvGetMutBuckets(iter);
		MOMO_CHECK(buckets != nullptr);
		(void)newKey;
		MOMO_EXTRA_CHECK(GetHashTraits().IsEqual(ItemTraits::GetKey(*iter), newKey));
		Bucket& bucket = (*buckets)[ConstIteratorProxy::GetBucketIndex(iter)];
		Item* bucketBegin = bucket.GetBounds(buckets->GetBucketParams()).GetBegin();
		return bucketBegin[std::addressof(*iter) - bucketBegin];
	}

	template<bool extraCheck, typename ItemCreator>
	InsertResult pvInsert(const Key& key, const ItemCreator& itemCreator)
	{
		ConstIterator iter = Find(key);
		if (!!iter)
			return InsertResult(iter, false);
		iter = pvAdd<extraCheck>(iter, itemCreator);
		return InsertResult(iter, true);
	}

	template<bool extraCheck, typename ItemCreator>
	ConstIterator pvAdd(ConstIterator iter, const ItemCreator& itemCreator)
	{
		ConstIteratorProxy::Check(iter, mCrew.GetVersion(), true);
		MOMO_CHECK(ConstIteratorProxy::GetBuckets(iter) == mBuckets);
		size_t hashCode = ConstIteratorProxy::GetHashCode(iter);
		Item* pitem;
		size_t bucketIndex;
		if (mCount < mCapacity)
			pitem = pvAddNogrow(hashCode, itemCreator, bucketIndex);
		else
			pitem = pvAddGrow(hashCode, itemCreator, bucketIndex);
		if (mBuckets->GetNextBuckets() != nullptr)
		{
			BucketParams& bucketParams = mBuckets->GetBucketParams();
			Bucket& bucket = (*mBuckets)[bucketIndex];
			size_t itemIndex = pitem - bucket.GetBounds(bucketParams).GetBegin();
			pvMoveItems();
			pitem = bucket.GetBounds(bucketParams).GetBegin() + itemIndex;
		}
		++mCount;
		mCrew.IncVersion();
		ConstIterator resIter = pvMakeIterator(*mBuckets, bucketIndex, pitem, false);
		MOMO_EXTRA_CHECK(!extraCheck || pvExtraCheck(resIter));
		return resIter;
	}

	template<typename ItemCreator>
	Item* pvAddNogrow(size_t hashCode, const ItemCreator& itemCreator, size_t& bucketIndex)
	{
		bucketIndex = pvGetBucketIndexForAdd(*mBuckets, hashCode);
		return (*mBuckets)[bucketIndex].AddBackCrt(mBuckets->GetBucketParams(), itemCreator);
	}

	template<typename ItemCreator>
	Item* pvAddGrow(size_t hashCode, const ItemCreator& itemCreator, size_t& bucketIndex)
	{
		const HashTraits& hashTraits = GetHashTraits();
		size_t newBucketCount = pvGetNewBucketCount();
		size_t newCapacity = hashTraits.CalcCapacity(newBucketCount);
		MOMO_CHECK(newCapacity > mCount);
		bool hasBuckets = (mBuckets != nullptr);
		Buckets* newBuckets;
		try
		{
			newBuckets = &Buckets::Create(GetMemManager(), newBucketCount,
				hasBuckets ? &mBuckets->GetBucketParams() : nullptr);
		}
		catch (...)	// std::bad_alloc&
		{
			if (Settings::overloadIfCannotGrow && hasBuckets)
				return pvAddNogrow(hashCode, itemCreator, bucketIndex);
			else
				throw;
		}
		Item* pitem;
		try
		{
			bucketIndex = pvGetBucketIndexForAdd(*newBuckets, hashCode);
			pitem = (*newBuckets)[bucketIndex].AddBackCrt(newBuckets->GetBucketParams(),
				itemCreator);
		}
		catch (...)
		{
			newBuckets->Destroy(GetMemManager(), !hasBuckets);
			throw;
		}
		newBuckets->SetNextBuckets(mBuckets);
		mBuckets = newBuckets;
		mCapacity = newCapacity;
		return pitem;
	}

	template<typename ReplaceFunc>
	ConstIterator pvRemove(ConstIterator iter, ReplaceFunc replaceFunc)
	{
		ConstIteratorProxy::Check(iter, mCrew.GetVersion(), false);
		Buckets* buckets = pvGetMutBuckets(iter);
		MOMO_CHECK(buckets != nullptr);
		BucketParams& bucketParams = buckets->GetBucketParams();
		size_t bucketIndex = ConstIteratorProxy::GetBucketIndex(iter);
		Bucket& bucket = (*buckets)[bucketIndex];
		typename Bucket::Bounds bucketBounds = bucket.GetBounds(bucketParams);
		Item* bucketBegin = bucketBounds.GetBegin();
		Item& bucketBack = *(bucketBounds.GetEnd() - 1);
		size_t itemIndex = std::addressof(*iter) - bucketBegin;
		replaceFunc(bucketBack, bucketBegin[itemIndex]);
		bucket.DecCount(bucketParams);
		--mCount;
		mCrew.IncVersion();
		if (!ConstIteratorProxy::IsMovable(iter))
			return ConstIterator();
		return pvMakeIterator(*buckets, bucketIndex,
			bucket.GetBounds(bucketParams).GetBegin() + itemIndex, true);
	}

	void pvMoveItems() MOMO_NOEXCEPT
	{
		Buckets* nextBuckets = mBuckets->GetNextBuckets();
		MOMO_ASSERT(nextBuckets != nullptr);
		try
		{
			pvMoveItems(nextBuckets);
			mBuckets->ExtractNextBuckets();
		}
		catch (...)
		{
			// no throw!
		}
	}

	void pvMoveItems(Buckets* buckets)
	{
		Buckets* nextBuckets = buckets->GetNextBuckets();
		if (nextBuckets != nullptr)
		{
			pvMoveItems(nextBuckets);
			buckets->ExtractNextBuckets();
		}
		const HashTraits& hashTraits = GetHashTraits();
		BucketParams& bucketParams = buckets->GetBucketParams();
		for (Bucket& bucket : *buckets)
		{
			typename Bucket::Bounds bucketBounds = bucket.GetBounds(bucketParams);
			for (Item* pitem = bucketBounds.GetEnd(); pitem != bucketBounds.GetBegin(); )
			{
				--pitem;
				size_t hashCode = hashTraits.GetHashCode(ItemTraits::GetKey(*pitem));
				size_t bucketIndex = pvGetBucketIndexForAdd(*mBuckets, hashCode);
				auto relocateCreator = [this, pitem] (Item* newItem)
					{ ItemTraits::Relocate(GetMemManager(), *pitem, newItem); };
				(*mBuckets)[bucketIndex].AddBackCrt(bucketParams, relocateCreator);
				bucket.DecCount(bucketParams);
			}
		}
		buckets->Destroy(GetMemManager(), false);
	}

private:
	Crew mCrew;
	size_t mCount;
	size_t mCapacity;
	Buckets* mBuckets;
};

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

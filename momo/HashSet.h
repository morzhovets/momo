/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/HashSet.h

  namespace momo:
    struct HashSetItemTraits
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
		typedef typename Bucket::Params BucketParams;
		typedef typename Bucket::ConstBounds ConstBucketBounds;

		static const size_t maxBucketCount =
			(SIZE_MAX - sizeof(size_t) - 2 * sizeof(void*)) / sizeof(Bucket);

	public:
		HashSetBuckets() = delete;

		HashSetBuckets(const HashSetBuckets&) = delete;

		~HashSetBuckets() = delete;

		HashSetBuckets& operator=(const HashSetBuckets&) = delete;

		template<typename MemManager>
		static HashSetBuckets& Create(MemManager& memManager, size_t bucketCount,
			BucketParams* bucketParams)
		{
			if (bucketCount > maxBucketCount)
				throw std::length_error("momo::internal::HashSetBuckets length error");
			size_t bufferSize = _GetBufferSize(bucketCount);
			HashSetBuckets& resBuckets = *memManager.template Allocate<HashSetBuckets>(bufferSize);
			resBuckets.mCount = 0;
			resBuckets.mNextBuckets = nullptr;
			Bucket* buckets = resBuckets._GetBuckets();
			try
			{
				size_t& curBucketCount = resBuckets.mCount;
				for (; curBucketCount < bucketCount; ++curBucketCount)
					new(buckets + curBucketCount) Bucket();
				if (bucketParams == nullptr)
					resBuckets.mBucketParams = &_CreateBucketParams(memManager);
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

		template<typename MemManager>
		void Destroy(MemManager& memManager, bool destroyBucketParams) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mNextBuckets == nullptr);
			size_t bucketCount = GetCount();
			Bucket* buckets = _GetBuckets();
			for (size_t i = 0; i < bucketCount; ++i)
				buckets[i].~Bucket();
			if (destroyBucketParams)
			{
				mBucketParams->~BucketParams();
				memManager.Deallocate(mBucketParams, sizeof(BucketParams));
			}
			memManager.Deallocate(this, _GetBufferSize(bucketCount));
		}

		Bucket* GetBegin() MOMO_NOEXCEPT
		{
			return _GetBuckets();
		}

		Bucket* GetEnd() MOMO_NOEXCEPT
		{
			return _GetBuckets() + GetCount();
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
			return _GetBuckets()[index];
		}

		Bucket& operator[](size_t index) MOMO_NOEXCEPT
		{
			return _GetBuckets()[index];
		}

		ConstBucketBounds GetBucketBounds(size_t index) const MOMO_NOEXCEPT
		{
			const BucketParams& bucketParams = *mBucketParams;
			return _GetBuckets()[index].GetBounds(bucketParams);
		}

		BucketParams& GetBucketParams() MOMO_NOEXCEPT
		{
			return *mBucketParams;
		}

	private:
		const Bucket* _GetBuckets() const MOMO_NOEXCEPT
		{
			return reinterpret_cast<const Bucket*>(this + 1);
		}

		Bucket* _GetBuckets() MOMO_NOEXCEPT
		{
			return reinterpret_cast<Bucket*>(this + 1);
		}

		static size_t _GetBufferSize(size_t bucketCount) MOMO_NOEXCEPT
		{
			return sizeof(HashSetBuckets) + bucketCount * sizeof(Bucket);
		}

		template<typename MemManager>
		static BucketParams& _CreateBucketParams(MemManager& memManager)
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
	public:
		typedef TBuckets Buckets;
		typedef TSettings Settings;
		typedef typename Buckets::Bucket Bucket;
		typedef typename Bucket::Item Item;
		typedef typename Bucket::ConstBounds ConstBucketBounds;

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

		HashSetConstIterator(const Buckets& buckets, size_t bucketIndex, const Item* pitem,
			const size_t* version, bool movable) MOMO_NOEXCEPT
			: IteratorVersion(version),
			mBuckets(&buckets),
			mBucketIndex(bucketIndex + (movable ? 0 : buckets.GetCount())),
			mItemPtr(pitem)
		{
			if (movable)
				_Move();
		}

		HashSetConstIterator(const Buckets* buckets, size_t hashCode,
			const size_t* version) MOMO_NOEXCEPT
			: IteratorVersion(version),
			mBuckets(buckets),
			mHashCode(hashCode),
			mItemPtr(nullptr)
		{
		}

		//operator ConstIterator() const MOMO_NOEXCEPT

		HashSetConstIterator& operator++()
		{
			MOMO_CHECK(mItemPtr != nullptr);
			MOMO_CHECK(IteratorVersion::Check());
			if (IsMovable())
			{
				++mItemPtr;
				_Move();
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

		bool IsMovable() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mItemPtr != nullptr && mBuckets != nullptr);
			return mBucketIndex < mBuckets->GetCount();
		}

		size_t GetBucketIndex() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mItemPtr != nullptr && mBuckets != nullptr);
			size_t bucketCount = mBuckets->GetCount();
			return (mBucketIndex < bucketCount) ? mBucketIndex : mBucketIndex - bucketCount;
		}

		size_t GetHashCode() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mItemPtr == nullptr);
			return mHashCode;
		}

		const Buckets* GetBuckets() const MOMO_NOEXCEPT
		{
			return mBuckets;
		}

		void Check(const size_t* version, bool empty) const
		{
			(void)version;
			(void)empty;
			MOMO_CHECK(empty || mBuckets != nullptr);
			MOMO_CHECK(empty ^ (mItemPtr != nullptr));
			MOMO_CHECK(IteratorVersion::Check(version));
		}

	private:
		void _Move() MOMO_NOEXCEPT
		{
			if (mItemPtr != _GetBucketBounds().GetEnd())
				return;
			size_t bucketCount = mBuckets->GetCount();
			while (true)
			{
				++mBucketIndex;
				if (mBucketIndex >= bucketCount)
					break;
				ConstBucketBounds bounds = _GetBucketBounds();
				mItemPtr = bounds.GetBegin();
				if (mItemPtr != bounds.GetEnd())
					return;
			}
			const Buckets* nextBuckets = mBuckets->GetNextBuckets();
			if (nextBuckets != nullptr)
			{
				mBuckets = nextBuckets;
				mBucketIndex = 0;
				mItemPtr = _GetBucketBounds().GetBegin();
				return _Move();	//?
			}
			*this = HashSetConstIterator();
		}

		ConstBucketBounds _GetBucketBounds() const MOMO_NOEXCEPT
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

	template<typename TItemTraits>
	struct HashSetBucketItemTraits
	{
		typedef TItemTraits ItemTraits;
		typedef typename ItemTraits::Item Item;

		static const size_t alignment = ItemTraits::alignment;

		static void Destroy(Item* items, size_t count) MOMO_NOEXCEPT
		{
			for (size_t i = 0; i < count; ++i)
				ItemTraits::Destroy(items[i]);
		}

		template<typename ItemCreator>
		static void RelocateCreate(Item* srcItems, Item* dstItems, size_t count,
			const ItemCreator& itemCreator, Item* newItem)
		{
			ItemTraits::RelocateCreate(srcItems, dstItems, count, itemCreator, newItem);
		}
	};
}

template<typename TKey,
	typename TItem = TKey>
struct HashSetItemTraits : public internal::SetItemTraits<TKey, TItem>
{
	typedef TKey Key;
	typedef TItem Item;

	typedef internal::ObjectManager<Item> ItemManager;

	template<typename ItemCreator>
	static void RelocateCreate(Item* srcItems, Item* dstItems, size_t count,
		const ItemCreator& itemCreator, Item* newItem)
	{
		ItemManager::RelocateCreate(srcItems, dstItems, count, itemCreator, newItem);
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
	typename TItemTraits = HashSetItemTraits<TKey>,
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
	typedef typename HashBucket::template Bucket<BucketItemTraits, MemManager> Bucket;

	typedef typename Bucket::Params BucketParams;

	typedef internal::HashSetBuckets<Bucket> Buckets;

	typedef internal::ArrayPtrIterator<const Item, Settings> ConstBucketIterator;

	template<typename... ItemArgs>
	using Creator = typename ItemTraits::template Creator<ItemArgs...>;

public:
	typedef internal::HashSetConstIterator<Buckets, Settings> ConstIterator;

	typedef internal::InsertResult<ConstIterator> InsertResult;

	typedef internal::SetExtractedItem<ItemTraits, Settings> ExtractedItem;

	typedef internal::HashDerivedBucketBounds<ConstBucketIterator,
		typename Bucket::ConstBounds> ConstBucketBounds;

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
			_Destroy();
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
				size_t bucketIndex = _GetBucketIndexForAdd(*mBuckets, hashCode);
				(*mBuckets)[bucketIndex].AddBackCrt(bucketParams, Creator<const Item&>(item));
			}
		}
		catch (...)
		{
			_Destroy();
			throw;
		}
	}

	~HashSet() MOMO_NOEXCEPT
	{
		_Destroy();
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
		if (mBuckets == nullptr)
			return ConstIterator();
		return _MakeIterator(*mBuckets, 0, mBuckets->GetBucketBounds(0).GetBegin(), true);
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
			_Destroy();
			mBuckets = nullptr;
			mCapacity = 0;
		}
		else
		{
			_Destroy(mBuckets->ExtractNextBuckets(), false);
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
		size_t newBucketCount = _GetNewBucketCount();
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
			_MoveItems();
	}

	void Shrink()
	{
		HashSet(*this).Swap(*this);
	}

	ConstIterator Find(const Key& key) const
	{
		return _Find(key);
	}

	template<typename KeyArg,
		bool isValidKeyArg = HashTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, ConstIterator>::type Find(const KeyArg& key) const
	{
		return _Find(key);
	}

	bool HasKey(const Key& key) const
	{
		return !!_Find(key);
	}

	template<typename KeyArg,
		bool isValidKeyArg = HashTraits::template IsValidKeyArg<KeyArg>::value>
	typename std::enable_if<isValidKeyArg, bool>::type HasKey(const KeyArg& key) const
	{
		return !!_Find(key);
	}

	template<typename ItemCreator>
	InsertResult InsertCrt(const Key& key, const ItemCreator& itemCreator)
	{
		return _Insert(key, itemCreator, true);
	}

	template<typename... ItemArgs>
	InsertResult InsertVar(const Key& key, ItemArgs&&... itemArgs)
	{
		return InsertCrt(key, Creator<ItemArgs...>(std::forward<ItemArgs>(itemArgs)...));
	}

	InsertResult Insert(Item&& item)
	{
		return _Insert(ItemTraits::GetKey(static_cast<const Item&>(item)),
			Creator<Item>(std::move(item)), false);
	}

	InsertResult Insert(const Item& item)
	{
		return _Insert(ItemTraits::GetKey(item), Creator<const Item&>(item), false);
	}

	template<typename Iterator>
	size_t Insert(Iterator begin, Iterator end)
	{
		MOMO_CHECK_TYPE(Item, *begin);
		size_t count = 0;
		for (Iterator iter = begin; iter != end; ++iter)
			count += Insert(*iter).inserted ? 1 : 0;
		return count;
	}

	size_t Insert(std::initializer_list<Item> items)
	{
		return Insert(items.begin(), items.end());
	}

	template<typename ItemCreator>
	ConstIterator AddCrt(ConstIterator iter, const ItemCreator& itemCreator)
	{
		return _Add(iter, itemCreator, true);
	}

	template<typename... ItemArgs>
	ConstIterator AddVar(ConstIterator iter, ItemArgs&&... itemArgs)
	{
		return AddCrt(iter, Creator<ItemArgs...>(std::forward<ItemArgs>(itemArgs)...));
	}

	ConstIterator Add(ConstIterator iter, Item&& item)
	{
		return AddVar(iter, std::move(item));
	}

	ConstIterator Add(ConstIterator iter, const Item& item)
	{
		return AddVar(iter, item);
	}

	ConstIterator Remove(ConstIterator iter)
	{
		auto replaceFunc = [] (Item& srcItem, Item& dstItem)
			{ ItemTraits::Replace(srcItem, dstItem); };
		return _Remove(iter, replaceFunc);
	}

	ConstIterator Remove(ConstIterator iter, ExtractedItem& resItem)
	{
		MOMO_CHECK(resItem.IsEmpty());
		auto replaceFunc = [&resItem] (Item& srcItem, Item& dstItem)
		{
			auto itemCreator = [&srcItem, &dstItem] (Item* newItem)
				{ ItemTraits::Replace(srcItem, dstItem, newItem); };
			resItem.SetItemCrt(itemCreator);
		};
		return _Remove(iter, replaceFunc);
	}

	bool Remove(const Key& key)
	{
		ConstIterator iter = Find(key);
		if (!iter)
			return false;
		Remove(iter);
		return true;
	}

	void ResetKey(ConstIterator iter, Key&& newKey)
	{
		Item& item = _GetItemForReset(iter, static_cast<const Key&>(newKey));
		ItemTraits::AssignKey(std::move(newKey), item);
	}

	void ResetKey(ConstIterator iter, const Key& newKey)
	{
		Item& item = _GetItemForReset(iter, newKey);
		ItemTraits::AssignKey(newKey, item);
	}

	template<typename Set>
	void MergeFrom(Set& srcSet)
	{
		MOMO_STATIC_ASSERT((std::is_same<Item, typename Set::Item>::value));
		auto insertFunc = [this] (Item&& item) { Insert(std::move(item)); };
		srcSet.MergeTo(insertFunc);
	}

	template<typename InsertFunc>
	void MergeTo(const InsertFunc& insertFunc)
	{
		for (Buckets* buckets = mBuckets; buckets != nullptr;
			buckets = buckets->GetNextBuckets())
		{
			BucketParams& bucketParams = buckets->GetBucketParams();
			for (Bucket& bucket : *buckets)
			{
				typename Bucket::Bounds bucketBounds = bucket.GetBounds(bucketParams);
				for (Item* pitem = bucketBounds.GetEnd(); pitem != bucketBounds.GetBegin(); )
				{
					--pitem;
					insertFunc(std::move(*pitem));	//?
					ItemTraits::Destroy(*pitem);
					bucket.DecCount(bucketParams);
					--mCount;
					mCrew.IncVersion();
				}
			}
		}
	}

	size_t GetBucketCount() const MOMO_NOEXCEPT
	{
		size_t bucketCount = 0;
		for (const Buckets* buckets = mBuckets; buckets != nullptr;
			buckets = buckets->GetNextBuckets())
		{
			bucketCount += buckets->GetCount();
		}
		return bucketCount;
	}

	ConstBucketBounds GetBucketBounds(size_t bucketIndex) const
	{
		MOMO_CHECK(bucketIndex < GetBucketCount());
		size_t curBucketIndex = bucketIndex;
		for (const Buckets* buckets = mBuckets; buckets != nullptr;
			buckets = buckets->GetNextBuckets())
		{
			size_t curBucketCount = buckets->GetCount();
			if (curBucketIndex < curBucketCount)
				return ConstBucketBounds(buckets->GetBucketBounds(curBucketIndex));
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
			return _GetBucketIndexForAdd(*mBuckets, iter.GetHashCode());	//?
		size_t bucketIndex = iter.GetBucketIndex();
		for (const Buckets* buckets = mBuckets; buckets != iter.GetBuckets();
			buckets = buckets->GetNextBuckets())
		{
			bucketIndex += buckets->GetCount();
		}
		return bucketIndex;
	}

private:
	void _Destroy() MOMO_NOEXCEPT
	{
		_Destroy(mBuckets, true);
	}

	void _Destroy(Buckets* buckets, bool destroyBucketParams) MOMO_NOEXCEPT
	{
		if (buckets == nullptr)
			return;
		BucketParams& bucketParams = buckets->GetBucketParams();
		for (Bucket& bucket : *buckets)
			bucket.Clear(bucketParams);
		_Destroy(buckets->ExtractNextBuckets(), false);
		buckets->Destroy(GetMemManager(), destroyBucketParams);
	}

	size_t _GetNewBucketCount() const
	{
		const HashTraits& hashTraits = GetHashTraits();
		if (mBuckets == nullptr)
			return (size_t)1 << hashTraits.GetLogStartBucketCount();
		size_t bucketCount = mBuckets->GetCount();
		size_t shift = hashTraits.GetBucketCountShift(bucketCount);
		MOMO_CHECK(shift > 0);
		return bucketCount << shift;
	}

	ConstIterator _MakeIterator(const Buckets& buckets, size_t bucketIndex,
		const Item* pitem, bool movable) const MOMO_NOEXCEPT
	{
		return ConstIterator(buckets, bucketIndex, pitem, mCrew.GetVersion(), movable);
	}

	bool _ExtraCheck(ConstIterator iter) const MOMO_NOEXCEPT
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
	ConstIterator _Find(const KeyArg& key) const
	{
		const HashTraits& hashTraits = GetHashTraits();
		size_t hashCode = hashTraits.GetHashCode(key);
		for (const Buckets* buckets = mBuckets; buckets != nullptr;
			buckets = buckets->GetNextBuckets())
		{
			const BucketParams& bucketParams = mBuckets->GetBucketParams();
			size_t bucketCount = buckets->GetCount();
			for (size_t probe = 0; probe < bucketCount; ++probe)
			{
				size_t bucketIndex = _GetBucketIndex(hashCode, bucketCount, probe);
				const Bucket& bucket = (*buckets)[bucketIndex];
				for (const Item& item : bucket.GetBounds(bucketParams))
				{
					if (hashTraits.IsEqual(key, ItemTraits::GetKey(item)))
						return _MakeIterator(*buckets, bucketIndex, std::addressof(item), false);
				}
				if (!bucket.WasFull())
					break;
			}
		}
		return ConstIterator(mBuckets, hashCode, mCrew.GetVersion());
	}

	size_t _GetBucketIndex(size_t hashCode, size_t bucketCount, size_t probe) const
	{
		const HashTraits& hashTraits = GetHashTraits();
		size_t bucketIndex = hashTraits.GetBucketIndex(hashCode, bucketCount, probe);
		MOMO_ASSERT(bucketIndex < bucketCount);
		return bucketIndex;
	}

	size_t _GetBucketIndexForAdd(const Buckets& buckets, size_t hashCode) const
	{
		size_t bucketCount = buckets.GetCount();
		for (size_t probe = 0; probe < bucketCount; ++probe)
		{
			size_t bucketIndex = _GetBucketIndex(hashCode, bucketCount, probe);
			if (!buckets[bucketIndex].IsFull())
				return bucketIndex;
		}
		throw std::runtime_error("momo::HashSet is full");
	}

	Buckets* _GetMutBuckets(ConstIterator iter) MOMO_NOEXCEPT
	{
		const Buckets* iterBuckets = iter.GetBuckets();
		for (Buckets* buckets = mBuckets; buckets != nullptr;
			buckets = buckets->GetNextBuckets())
		{
			if (buckets == iterBuckets)
				return buckets;
		}
		return nullptr;
	}

	Item& _GetItemForReset(ConstIterator iter, const Key& newKey)
	{
		iter.Check(mCrew.GetVersion(), false);
		Buckets* buckets = _GetMutBuckets(iter);
		MOMO_CHECK(buckets != nullptr);
		(void)newKey;
		MOMO_EXTRA_CHECK(GetHashTraits().IsEqual(ItemTraits::GetKey(*iter), newKey));
		Bucket& bucket = (*buckets)[iter.GetBucketIndex()];
		Item* bucketBegin = bucket.GetBounds(buckets->GetBucketParams()).GetBegin();
		return bucketBegin[std::addressof(*iter) - bucketBegin];
	}

	template<typename ItemCreator>
	InsertResult _Insert(const Key& key, const ItemCreator& itemCreator, bool extraCheck)
	{
		ConstIterator iter = Find(key);
		if (!!iter)
			return InsertResult(iter, false);
		iter = _Add(iter, itemCreator, extraCheck);
		return InsertResult(iter, true);
	}

	template<typename ItemCreator>
	ConstIterator _Add(ConstIterator iter, const ItemCreator& itemCreator, bool extraCheck)
	{
		iter.Check(mCrew.GetVersion(), true);
		MOMO_CHECK(iter.GetBuckets() == mBuckets);
		size_t hashCode = iter.GetHashCode();
		Item* pitem;
		size_t bucketIndex;
		if (mCount < mCapacity)
			pitem = _AddNogrow(hashCode, itemCreator, bucketIndex);
		else
			pitem = _AddGrow(hashCode, itemCreator, bucketIndex);
		if (mBuckets->GetNextBuckets() != nullptr)
		{
			BucketParams& bucketParams = mBuckets->GetBucketParams();
			Bucket& bucket = (*mBuckets)[bucketIndex];
			size_t itemIndex = pitem - bucket.GetBounds(bucketParams).GetBegin();
			_MoveItems();
			pitem = bucket.GetBounds(bucketParams).GetBegin() + itemIndex;
		}
		++mCount;
		mCrew.IncVersion();
		ConstIterator resIter = _MakeIterator(*mBuckets, bucketIndex, pitem, false);
		(void)extraCheck;
		MOMO_EXTRA_CHECK(!extraCheck || _ExtraCheck(resIter));
		return resIter;
	}

	template<typename ItemCreator>
	Item* _AddNogrow(size_t hashCode, const ItemCreator& itemCreator, size_t& bucketIndex)
	{
		bucketIndex = _GetBucketIndexForAdd(*mBuckets, hashCode);
		return (*mBuckets)[bucketIndex].AddBackCrt(mBuckets->GetBucketParams(), itemCreator);
	}

	template<typename ItemCreator>
	Item* _AddGrow(size_t hashCode, const ItemCreator& itemCreator, size_t& bucketIndex)
	{
		const HashTraits& hashTraits = GetHashTraits();
		size_t newBucketCount = _GetNewBucketCount();
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
				return _AddNogrow(hashCode, itemCreator, bucketIndex);
			else
				throw;
		}
		Item* pitem;
		try
		{
			bucketIndex = _GetBucketIndexForAdd(*newBuckets, hashCode);
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
	ConstIterator _Remove(ConstIterator iter, ReplaceFunc replaceFunc)
	{
		iter.Check(mCrew.GetVersion(), false);
		Buckets* buckets = _GetMutBuckets(iter);
		MOMO_CHECK(buckets != nullptr);
		BucketParams& bucketParams = buckets->GetBucketParams();
		size_t bucketIndex = iter.GetBucketIndex();
		Bucket& bucket = (*buckets)[bucketIndex];
		typename Bucket::Bounds bucketBounds = bucket.GetBounds(bucketParams);
		Item* bucketBegin = bucketBounds.GetBegin();
		Item& bucketBack = *(bucketBounds.GetEnd() - 1);
		size_t itemIndex = std::addressof(*iter) - bucketBegin;
		replaceFunc(bucketBack, bucketBegin[itemIndex]);
		bucket.DecCount(bucketParams);
		--mCount;
		mCrew.IncVersion();
		if (!iter.IsMovable())
			return ConstIterator();
		return _MakeIterator(*buckets, bucketIndex,
			bucket.GetBounds(bucketParams).GetBegin() + itemIndex, true);
	}

	void _MoveItems() MOMO_NOEXCEPT
	{
		Buckets* nextBuckets = mBuckets->GetNextBuckets();
		MOMO_ASSERT(nextBuckets != nullptr);
		try
		{
			_MoveItems(nextBuckets);
			mBuckets->ExtractNextBuckets();
		}
		catch (...)
		{
			// no throw!
		}
	}

	void _MoveItems(Buckets* buckets)
	{
		Buckets* nextBuckets = buckets->GetNextBuckets();
		if (nextBuckets != nullptr)
		{
			_MoveItems(nextBuckets);
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
				size_t bucketIndex = _GetBucketIndexForAdd(*mBuckets, hashCode);
				auto relocateCreator = [pitem] (Item* newItem)
					{ ItemTraits::Relocate(*pitem, newItem); };
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
		typedef typename momo::internal::HashSetConstIterator<B, S>::Item value_type;
	};
} // namespace std

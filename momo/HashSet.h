/**********************************************************\

  momo/HashSet.h

  namespace momo:
    struct HashSetItemTraits
    struct HashSetSettings
    class HashSet

\**********************************************************/

#pragma once

#include "HashUtility.h"
#include "HashTraits.h"

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
			const BucketParams& bucketParams)
		{
			if (bucketCount > maxBucketCount)
				throw std::length_error("momo::internal::HashSetBuckets length error");
			size_t bufferSize = _GetBufferSize(bucketCount);
			HashSetBuckets& resBuckets = *(HashSetBuckets*)memManager.Allocate(bufferSize);
			resBuckets.mCount = 0;
			resBuckets.mNextBuckets = nullptr;
			resBuckets.mBucketParams = &bucketParams;
			Bucket* buckets = resBuckets._GetBuckets();
			try
			{
				size_t& curBucketCount = resBuckets.mCount;
				for (; curBucketCount < bucketCount; ++curBucketCount)
					new(buckets + curBucketCount) Bucket();
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
		void Destroy(MemManager& memManager) MOMO_NOEXCEPT
		{
			assert(mNextBuckets == nullptr);
			size_t bucketCount = GetCount();
			Bucket* buckets = _GetBuckets();
			for (size_t i = 0; i < bucketCount; ++i)
				buckets[i].~Bucket();
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
			assert(mNextBuckets == nullptr);
			mNextBuckets = nextBuckets;
		}

		size_t GetCount() const MOMO_NOEXCEPT
		{
			return mCount;
		}

		const Bucket& operator[](size_t index) const MOMO_NOEXCEPT
		{
			return _GetBucket(index);
		}

		Bucket& operator[](size_t index) MOMO_NOEXCEPT
		{
			return _GetBucket(index);
		}

		ConstBucketBounds GetBucketBounds(size_t index) const MOMO_NOEXCEPT
		{
			return _GetBucket(index).GetBounds(*mBucketParams);
		}

	private:
		Bucket* _GetBuckets() const MOMO_NOEXCEPT
		{
			return (Bucket*)(this + 1);
		}

		Bucket& _GetBucket(size_t index) const MOMO_NOEXCEPT
		{
			return _GetBuckets()[index];
		}

		static size_t _GetBufferSize(size_t bucketCount) MOMO_NOEXCEPT
		{
			return sizeof(HashSetBuckets) + bucketCount * sizeof(Bucket);
		}

	private:
		size_t mCount;
		HashSetBuckets* mNextBuckets;
		union
		{
			const BucketParams* mBucketParams;
			typename std::aligned_storage<std::alignment_of<Bucket>::value,
				std::alignment_of<Bucket>::value>::type mBucketPadding;
		};
	};

	template<typename TBuckets, typename TSettings>
	class HashSetConstIterator : private HashIteratorVersion<TSettings::checkVersion>
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
		typedef HashIteratorVersion<Settings::checkVersion> HashIteratorVersion;

	public:
		HashSetConstIterator() MOMO_NOEXCEPT
			: mBuckets(nullptr),
			mHashCode(0),
			mItemPtr(nullptr)
		{
		}

		HashSetConstIterator(const Buckets& buckets, size_t bucketIndex, const Item* pitem,
			const size_t& version, bool move) MOMO_NOEXCEPT
			: HashIteratorVersion(version),
			mBuckets(&buckets),
			mBucketIndex(bucketIndex),
			mItemPtr(pitem)
		{
			if (move)
				_Move();
		}

		HashSetConstIterator(const Buckets& buckets, size_t hashCode,
			const size_t& version) MOMO_NOEXCEPT
			: HashIteratorVersion(version),
			mBuckets(&buckets),
			mHashCode(hashCode),
			mItemPtr(nullptr)
		{
		}

		//operator ConstIterator() const MOMO_NOEXCEPT

		HashSetConstIterator& operator++()
		{
			MOMO_CHECK(mItemPtr != nullptr);
			MOMO_CHECK(HashIteratorVersion::Check());
			++mItemPtr;
			_Move();
			return *this;
		}

		Pointer operator->() const
		{
			MOMO_CHECK(mItemPtr != nullptr);
			MOMO_CHECK(HashIteratorVersion::Check());
			return mItemPtr;
		}

		bool operator==(ConstIterator iter) const MOMO_NOEXCEPT
		{
			return mItemPtr == iter.mItemPtr;
		}

		MOMO_MORE_HASH_ITERATOR_OPERATORS(HashSetConstIterator)

		size_t GetBucketIndex() const MOMO_NOEXCEPT
		{
			assert(mItemPtr != nullptr);
			return mBucketIndex;
		}

		size_t GetHashCode() const MOMO_NOEXCEPT
		{
			assert(mItemPtr == nullptr);
			return mHashCode;
		}

		const Buckets* GetBuckets() const MOMO_NOEXCEPT
		{
			return mBuckets;
		}

		void Check(const size_t& version, bool empty) const
		{
			(void)version;
			(void)empty;
			MOMO_CHECK(mBuckets != nullptr);
			MOMO_CHECK(empty ^ (mItemPtr != nullptr));
			MOMO_CHECK(HashIteratorVersion::Check(version));
		}

	private:
		void _Move() MOMO_NOEXCEPT
		{
			if (mItemPtr != mBuckets->GetBucketBounds(mBucketIndex).GetEnd())
				return;
			size_t bucketCount = mBuckets->GetCount();
			++mBucketIndex;
			for (; mBucketIndex < bucketCount; ++mBucketIndex)
			{
				ConstBucketBounds bounds = mBuckets->GetBucketBounds(mBucketIndex);
				mItemPtr = bounds.GetBegin();
				if (mItemPtr != bounds.GetEnd())
					return;
			}
			const Buckets* nextBuckets = mBuckets->GetNextBuckets();
			if (nextBuckets != nullptr)
			{
				mBuckets = nextBuckets;
				mBucketIndex = 0;
				mItemPtr = mBuckets->GetBucketBounds(0).GetBegin();
				return _Move();	//?
			}
			mBuckets = nullptr;
			mItemPtr = nullptr;
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
}

template<typename TKey,
	typename TItem = TKey>
struct HashSetItemTraits
{
	typedef TKey Key;
	typedef TItem Item;

	typedef internal::ObjectManager<Item> ItemManager;

	static const size_t alignment = ItemManager::alignment;

	template<typename... ItemArgs>
	using Creator = typename ItemManager::template Creator<ItemArgs...>;

	static const Key& GetKey(const Item& item) MOMO_NOEXCEPT
	{
		MOMO_STATIC_ASSERT((std::is_same<Item, Key>::value));
		return item;
	}

	static void Destroy(Item* items, size_t count) MOMO_NOEXCEPT
	{
		ItemManager::Destroy(items, count);
	}

	static void Assign(Item&& srcItem, Item& dstItem)
	{
		dstItem = std::move(srcItem);
	}

	static void Assign(const Item& srcItem, Item& dstItem)
	{
		dstItem = srcItem;
	}

	static void Assign(Item&& srcItem, Item& midItem, Item& dstItem)
	{
		_Assign(std::move(srcItem), midItem, dstItem,
			internal::BoolConstant<ItemManager::isNothrowAnywayMoveAssignable>());
	}

	static void Assign(const Item& srcItem, Item& midItem, Item& dstItem)
	{
		_Assign(srcItem, midItem, dstItem,
			internal::BoolConstant<ItemManager::isNothrowAnywayCopyAssignable>());
	}

	template<typename ItemCreator>
	static void RelocateAddBack(Item* srcItems, Item* dstItems, size_t srcCount,
		const ItemCreator& itemCreator)
	{
		ItemManager::RelocateAddBack(srcItems, dstItems, srcCount, itemCreator);
	}

private:
	static void _Assign(Item&& srcItem, Item& midItem, Item& dstItem,
		std::true_type /*isNothrowAnywayMoveAssignable*/)
	{
		dstItem = std::move(midItem);
		ItemManager::AssignNothrowAnyway(std::move(srcItem), midItem);
	}

	static void _Assign(Item&& srcItem, Item& midItem, Item& dstItem,
		std::false_type /*isNothrowAnywayMoveAssignable*/)
	{
		dstItem = midItem;
		midItem = std::move(srcItem);
	}

	static void _Assign(const Item& srcItem, Item& midItem, Item& dstItem,
		std::true_type /*isNothrowAnywayCopyAssignable*/)
	{
		dstItem = std::move(midItem);
		ItemManager::AssignNothrowAnyway(srcItem, midItem);
	}

	static void _Assign(const Item& srcItem, Item& midItem, Item& dstItem,
		std::false_type /*isNothrowAnywayCopyAssignable*/)
	{
		dstItem = midItem;
		midItem = srcItem;
	}
};

struct HashSetSettings
{
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;

	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION_VALUE;
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
	template<typename... ItemArgs>
	using Creator = typename ItemTraits::template Creator<ItemArgs...>;

	struct BucketItemTraits
	{
		typedef typename HashSet::Item Item;

		static const size_t alignment = ItemTraits::alignment;

		static void Destroy(Item* items, size_t count) MOMO_NOEXCEPT
		{
			ItemTraits::Destroy(items, count);
		}

		template<typename ItemCreator>
		static void RelocateAddBack(Item* srcItems, Item* dstItems, size_t srcCount,
			const ItemCreator& itemCreator)
		{
			ItemTraits::RelocateAddBack(srcItems, dstItems, srcCount, itemCreator);
		}
	};

	typedef typename HashTraits::HashBucket HashBucket;
	typedef typename HashBucket::template Bucket<BucketItemTraits, MemManager> Bucket;

	typedef typename Bucket::Params BucketParams;

	class Crew
	{
		MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<MemManager>::value);

	private:
		struct Data
		{
			size_t version;
			HashTraits hashTraits;
			BucketParams bucketParams;
			MemManager memManager;
		};

	public:
		Crew(const HashTraits& hashTraits, MemManager&& memManager)
		{
			mData = (Data*)memManager.Allocate(sizeof(Data));
			mData->version = 0;
			new(&mData->memManager) MemManager(std::move(memManager));
			try
			{
				new(&mData->hashTraits) HashTraits(hashTraits);
				try
				{
					new(&mData->bucketParams) BucketParams(mData->memManager);
				}
				catch (...)
				{
					mData->hashTraits.~HashTraits();
					throw;
				}
			}
			catch (...)
			{
				MemManager dataMemManager = std::move(mData->memManager);
				mData->memManager.~MemManager();
				dataMemManager.Deallocate(mData, sizeof(Data));
				throw;
			}
		}

		Crew(Crew&& crew) MOMO_NOEXCEPT
			: mData(nullptr)
		{
			Swap(crew);
		}

		Crew(const Crew&) = delete;

		~Crew() MOMO_NOEXCEPT
		{
			if (!_IsNull())
			{
				mData->bucketParams.~BucketParams();
				mData->hashTraits.~HashTraits();
				MemManager memManager = std::move(mData->memManager);
				mData->memManager.~MemManager();
				memManager.Deallocate(mData, sizeof(Data));
			}
		}

		Crew& operator=(Crew&& crew) MOMO_NOEXCEPT
		{
			Crew(std::move(crew)).Swap(*this);
			return *this;
		}

		Crew& operator=(const Crew&) = delete;

		void Swap(Crew& crew) MOMO_NOEXCEPT
		{
			std::swap(mData, crew.mData);
		}

		const size_t& GetVersion() const MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->version;
		}

		size_t& GetVersion() MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->version;
		}

		const BucketParams& GetBucketParams() const MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->bucketParams;
		}

		BucketParams& GetBucketParams() MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->bucketParams;
		}

		const HashTraits& GetHashTraits() const MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->hashTraits;
		}

		const MemManager& GetMemManager() const MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->memManager;
		}

		MemManager& GetMemManager() MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->memManager;
		}

	private:
		bool _IsNull() const MOMO_NOEXCEPT
		{
			return mData == nullptr;
		}

	private:
		Data* mData;
	};

	typedef internal::HashSetBuckets<Bucket> Buckets;

public:
	typedef internal::HashSetConstIterator<Buckets, Settings> ConstIterator;

	typedef internal::HashInsertResult<ConstIterator> InsertResult;

	typedef typename Bucket::ConstBounds ConstBucketBounds;

public:
	explicit HashSet(const HashTraits& hashTraits = HashTraits(),
		MemManager&& memManager = MemManager())
		: mCrew(hashTraits, std::move(memManager)),
		mCount(0)
	{
		size_t bucketCount = (size_t)1 << hashTraits.GetLogStartBucketCount();
		mCapacity = hashTraits.CalcCapacity(bucketCount);
		mBuckets = &Buckets::Create(GetMemManager(), bucketCount, mCrew.GetBucketParams());
	}

	HashSet(std::initializer_list<Item> items,
		const HashTraits& hashTraits = HashTraits(), MemManager&& memManager = MemManager())
		: HashSet(hashTraits, std::move(memManager))
	{
		Insert(items);
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
		: mCrew(hashSet.GetHashTraits(), MemManager(hashSet.GetMemManager())),
		mCount(hashSet.mCount)
	{
		const HashTraits& hashTraits = GetHashTraits();
		BucketParams& bucketParams = mCrew.GetBucketParams();
		size_t bucketCount = (size_t)1 << hashTraits.GetLogStartBucketCount();
		while (true)
		{
			mCapacity = hashTraits.CalcCapacity(bucketCount);
			if (mCapacity >= mCount)
				break;
			bucketCount <<= 1;
		}
		mBuckets = &Buckets::Create(GetMemManager(), bucketCount, bucketParams);
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
			_DestroyBuckets(mBuckets);
			throw;
		}
	}

	~HashSet() MOMO_NOEXCEPT
	{
		_DestroyBuckets(mBuckets);
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
		if (mBuckets == nullptr)	//?
			return ConstIterator();
		return _MakeIterator(*mBuckets, 0,
			(*mBuckets)[0].GetBounds(mCrew.GetBucketParams()).GetBegin(), true);
	}

	ConstIterator GetEnd() const MOMO_NOEXCEPT
	{
		return ConstIterator();
	}

	MOMO_FRIEND_SWAP(HashSet)
	MOMO_FRIENDS_BEGIN_END(const HashSet&, ConstIterator)

	const HashTraits& GetHashTraits() const MOMO_NOEXCEPT
	{
		return mCrew.GetHashTraits();
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

	void Clear() MOMO_NOEXCEPT
	{
		if (mBuckets == nullptr)	//?
			return;
		_DestroyBuckets(mBuckets->ExtractNextBuckets());
		BucketParams& bucketParams = mCrew.GetBucketParams();
		for (Bucket& bucket : *mBuckets)
			bucket.Clear(bucketParams);
		mCount = 0;
		++mCrew.GetVersion();
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
		size_t bucketCount = mBuckets->GetCount();
		size_t shift = hashTraits.GetBucketCountShift(bucketCount);
		//? MOMO_CHECK(shift > 0);
		size_t newBucketCount = bucketCount << shift;
		size_t newCapacity;
		while (true)
		{
			newCapacity = hashTraits.CalcCapacity(newBucketCount);
			if (newCapacity >= capacity)
				break;
			newBucketCount <<= 1;
		}
		Buckets& newBuckets = Buckets::Create(GetMemManager(), newBucketCount,
			mCrew.GetBucketParams());
		newBuckets.SetNextBuckets(mBuckets);
		mBuckets = &newBuckets;
		mCapacity = newCapacity;
		++mCrew.GetVersion();
		_MoveItems();
	}

	void Shrink()
	{
		HashSet(*this).Swap(*this);
	}

	ConstIterator Find(const Key& key) const
	{
		const HashTraits& hashTraits = GetHashTraits();
		const BucketParams& bucketParams = mCrew.GetBucketParams();
		size_t hashCode = hashTraits.GetHashCode(key);
		const Buckets* buckets = mBuckets;
		while (true)
		{
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
			buckets = buckets->GetNextBuckets();
			if (buckets == nullptr)
				break;
		}
		return ConstIterator(*mBuckets, hashCode, mCrew.GetVersion());
	}

	bool HasKey(const Key& key) const
	{
		return !!Find(key);
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
		return _Insert(ItemTraits::GetKey((const Item&)item),
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
		auto removeFunc = [] (Item& item, Item& backItem)
			{ ItemTraits::Assign(std::move(backItem), item); };
		return _Remove(iter, removeFunc);
	}

	ConstIterator Remove(ConstIterator iter, Item& resItem)
	{
		auto removeFunc = [&resItem] (Item& item, Item& backItem)
			{ ItemTraits::Assign(std::move(backItem), item, resItem); };
		return _Remove(iter, removeFunc);
	}

	bool Remove(const Key& key)
	{
		ConstIterator iter = Find(key);
		if (!iter)
			return false;
		Remove(iter);
		return true;
	}

	void Reset(ConstIterator iter, Item&& newItem)
	{
		Item& item = _GetItemForReset(iter, (const Item&)newItem);
		ItemTraits::Assign(std::move(newItem), item);
	}

	void Reset(ConstIterator iter, const Item& newItem)
	{
		Item& item = _GetItemForReset(iter, newItem);
		ItemTraits::Assign(newItem, item);
	}

	void Reset(ConstIterator iter, Item&& newItem, Item& resItem)
	{
		Item& item = _GetItemForReset(iter, (const Item&)newItem);
		ItemTraits::Assign(std::move(newItem), item, resItem);
	}

	void Reset(ConstIterator iter, const Item& newItem, Item& resItem)
	{
		Item& item = _GetItemForReset(iter, newItem);
		ItemTraits::Assign(newItem, item, resItem);
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
				return (*buckets)[curBucketIndex].GetBounds(mCrew.GetBucketParams());
			curBucketIndex -= curBucketCount;
		}
		assert(false);
		return ConstBucketBounds();
	}

	size_t GetBucketIndex(const Key& key) const
	{
		ConstIterator iter = Find(key);
		if (!!iter)
			return iter.GetBucketIndex();
		return _GetBucketIndexForAdd(*mBuckets, iter.GetHashCode());
	}

private:
	void _DestroyBuckets(Buckets* buckets) MOMO_NOEXCEPT
	{
		while (buckets != nullptr)	//?
		{
			BucketParams& bucketParams = mCrew.GetBucketParams();
			for (Bucket& bucket : *buckets)
				bucket.Clear(bucketParams);
			Buckets* nextBuckets = buckets->ExtractNextBuckets();
			buckets->Destroy(GetMemManager());
			buckets = nextBuckets;
		}
	}

	ConstIterator _MakeIterator(const Buckets& buckets, size_t bucketIndex,
		const Item* pitem, bool move) const MOMO_NOEXCEPT
	{
		return ConstIterator(buckets, bucketIndex, pitem, mCrew.GetVersion(), move);
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

	size_t _GetBucketIndex(size_t hashCode, size_t bucketCount, size_t probe) const
	{
		const HashTraits& hashTraits = GetHashTraits();
		size_t bucketIndex = hashTraits.GetBucketIndex(hashCode, bucketCount, probe);
		assert(bucketIndex < bucketCount);
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

	Item& _GetItemForReset(ConstIterator iter, const Item& newItem)
	{
		iter.Check(mCrew.GetVersion(), false);
		Buckets* buckets = _GetMutBuckets(iter);
		MOMO_CHECK(buckets != nullptr);
		(void)newItem;
		MOMO_EXTRA_CHECK(GetHashTraits().IsEqual(ItemTraits::GetKey(*iter),
			ItemTraits::GetKey(newItem)));
		Bucket& bucket = (*buckets)[iter.GetBucketIndex()];
		Item* bucketBegin = bucket.GetBounds(mCrew.GetBucketParams()).GetBegin();
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
		size_t bucketIndex;	//?
		if (mCount < mCapacity)
			bucketIndex = _AddNogrow(hashCode, itemCreator);
		else
			bucketIndex = _AddGrow(hashCode, itemCreator);
		Bucket& bucket = (*mBuckets)[bucketIndex];
		size_t itemIndex = bucket.GetBounds(mCrew.GetBucketParams()).GetCount() - 1;
		if (mBuckets->GetNextBuckets() != nullptr)
			_MoveItems();
		++mCount;
		++mCrew.GetVersion();
		ConstIterator resIter = _MakeIterator(*mBuckets, bucketIndex,
			bucket.GetBounds(mCrew.GetBucketParams()).GetBegin() + itemIndex, false);
		(void)extraCheck;
		MOMO_EXTRA_CHECK(!extraCheck || resIter == Find(ItemTraits::GetKey(*resIter)));
		return resIter;
	}

	template<typename ItemCreator>
	size_t _AddNogrow(size_t hashCode, const ItemCreator& itemCreator)
	{
		size_t bucketIndex = _GetBucketIndexForAdd(*mBuckets, hashCode);
		(*mBuckets)[bucketIndex].AddBackCrt(mCrew.GetBucketParams(), itemCreator);
		return bucketIndex;
	}

	template<typename ItemCreator>
	size_t _AddGrow(size_t hashCode, const ItemCreator& itemCreator)
	{
		const HashTraits& hashTraits = GetHashTraits();
		size_t bucketCount = mBuckets->GetCount();
		size_t shift = hashTraits.GetBucketCountShift(bucketCount);
		size_t newBucketCount = bucketCount << shift;
		size_t newCapacity = hashTraits.CalcCapacity(newBucketCount);
		MOMO_CHECK(newCapacity > mCount);
		Buckets* newBuckets;
		try
		{
			newBuckets = &Buckets::Create(GetMemManager(), newBucketCount,
				mCrew.GetBucketParams());
		}
		catch (...)	// std::bad_alloc&
		{
			return _AddNogrow(hashCode, itemCreator);
		}
		size_t bucketIndex;
		try
		{
			bucketIndex = _GetBucketIndexForAdd(*newBuckets, hashCode);
			(*newBuckets)[bucketIndex].AddBackCrt(mCrew.GetBucketParams(), itemCreator);
		}
		catch (...)
		{
			newBuckets->Destroy(GetMemManager());
			throw;
		}
		newBuckets->SetNextBuckets(mBuckets);
		mBuckets = newBuckets;
		mCapacity = newCapacity;
		return bucketIndex;
	}

	template<typename RemoveFunc>
	ConstIterator _Remove(ConstIterator iter, RemoveFunc removeFunc)
	{
		iter.Check(mCrew.GetVersion(), false);
		Buckets* buckets = _GetMutBuckets(iter);
		MOMO_CHECK(buckets != nullptr);
		size_t bucketIndex = iter.GetBucketIndex();
		Bucket& bucket = (*buckets)[bucketIndex];
		typename Bucket::Bounds bucketBounds = bucket.GetBounds(mCrew.GetBucketParams());
		Item* bucketBegin = bucketBounds.GetBegin();
		size_t itemIndex = std::addressof(*iter) - bucketBegin;
		removeFunc(bucketBegin[itemIndex], *(bucketBounds.GetEnd() - 1));
		bucket.RemoveBack(mCrew.GetBucketParams());
		--mCount;
		++mCrew.GetVersion();
		return _MakeIterator(*buckets, bucketIndex,
			bucket.GetBounds(mCrew.GetBucketParams()).GetBegin() + itemIndex, true);
	}

	void _MoveItems() MOMO_NOEXCEPT
	{
		Buckets* nextBuckets = mBuckets->GetNextBuckets();
		assert(nextBuckets != nullptr);
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
		BucketParams& bucketParams = mCrew.GetBucketParams();
		for (Bucket& bucket : *buckets)
		{
			while (true)
			{
				typename Bucket::Bounds bucketBounds = bucket.GetBounds(bucketParams);
				if (bucketBounds.GetCount() == 0)
					break;
				Item& item = *(bucketBounds.GetEnd() - 1);
				size_t hashCode = hashTraits.GetHashCode(ItemTraits::GetKey(item));
				size_t bucketIndex = _GetBucketIndexForAdd(*mBuckets, hashCode);
				(*mBuckets)[bucketIndex].AddBackCrt(bucketParams, Creator<Item>(std::move(item)));
				bucket.RemoveBack(bucketParams);
			}
		}
		for (Bucket& bucket : *buckets)
			bucket.Clear(bucketParams);
		buckets->Destroy(GetMemManager());
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

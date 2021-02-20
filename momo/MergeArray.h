/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/MergeArray.h

  namespace momo:
    class MergeArrayItemTraits
    class MergeArraySettings
    class MergeArray

\**********************************************************/

#pragma once

#include "Array.h"

namespace momo
{

template<typename TItem, conceptMemManager TMemManager>
class MergeArrayItemTraits
{
public:
	typedef TItem Item;
	typedef TMemManager MemManager;

private:
	typedef internal::ObjectManager<Item, MemManager> ItemManager;

public:
	static const size_t alignment = ItemManager::alignment;

	template<typename... ItemArgs>
	using Creator = typename ItemManager::template Creator<ItemArgs...>;

public:
	template<typename Iterator>
	static void Destroy(MemManager& memManager, Iterator begin, size_t count) noexcept
	{
		ItemManager::Destroy(memManager, begin, count);
	}

	template<typename SrcIterator, typename DstIterator>
	static void Relocate(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
		size_t count)
	{
		ItemManager::Relocate(memManager, srcBegin, dstBegin, count);
	}

	template<typename SrcIterator, typename DstIterator, typename ItemCreator>
	static void RelocateCreate(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
		size_t count, ItemCreator&& itemCreator, Item* newItem)
	{
		ItemManager::RelocateCreate(memManager, srcBegin, dstBegin, count,
			std::forward<ItemCreator>(itemCreator), newItem);
	}

	template<typename ItemArg>
	static void Assign(MemManager& /*memManager*/, ItemArg&& itemArg, Item& item)
	{
		item = std::forward<ItemArg>(itemArg);
	}
};

template<size_t tLogInitialItemCount = 5>
class MergeArraySettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;

	static const size_t logInitialItemCount = tLogInitialItemCount;

	typedef ArraySettings<0, false> SegmentsSettings;
};

/*!
	All `MergeArray` functions and constructors have strong exception safety,
	but not the following cases:
	 - Functions `Insert`, `InsertVar`, `InsertCrt`, `Remove` have
	basic exception safety.

	Swap and move operations invalidate all container iterators.
*/

template<typename TItem,
	conceptMemManager TMemManager = MemManagerDefault,
	typename TItemTraits = MergeArrayItemTraits<TItem, TMemManager>,
	typename TSettings = MergeArraySettings<>>
class MergeArray
{
public:
	typedef TItem Item;
	typedef TMemManager MemManager;
	typedef TItemTraits ItemTraits;
	typedef TSettings Settings;

	typedef internal::ArrayIndexIterator<MergeArray, Item> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

private:
	typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

	typedef internal::NestedArraySettings<typename Settings::SegmentsSettings> SegmentsSettings;

	typedef Array<Item*, MemManager, ArrayItemTraits<Item*, MemManager>,
		SegmentsSettings> Segments;

	typedef internal::ArrayItemHandler<ItemTraits> ItemHandler;
	typedef internal::ArrayShifter<MergeArray> ArrayShifter;

	static const size_t logInitialItemCount = Settings::logInitialItemCount;
	static const size_t initialItemCount = size_t{1} << logInitialItemCount;

	struct ConstIteratorProxy : public ConstIterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
	};

	struct IteratorProxy : public Iterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
	};

public:
	MergeArray() noexcept(noexcept(MemManager()))
		: MergeArray(MemManager())
	{
	}

	explicit MergeArray(MemManager memManager) noexcept
		: mSegments(std::move(memManager)),
		mCount(0),
		mCapacity(0)
	{
	}

	explicit MergeArray(size_t count, MemManager memManager = MemManager())
		: MergeArray(std::move(memManager))
	{
		pvInitCapacity(count);
		try
		{
			for (size_t i = 0; i < count; ++i)
				AddBackNogrowVar();
		}
		catch (...)
		{
			pvDestroy();
			throw;
		}
	}

	explicit MergeArray(size_t count, const Item& item, MemManager memManager = MemManager())
		: MergeArray(std::move(memManager))
	{
		pvInitCapacity(count);
		try
		{
			for (size_t i = 0; i < count; ++i)
				AddBackNogrow(item);
		}
		catch (...)
		{
			pvDestroy();
			throw;
		}
	}

	template<internal::conceptInputIterator ArgIterator>
	explicit MergeArray(ArgIterator begin, ArgIterator end, MemManager memManager = MemManager())
		: MergeArray(std::move(memManager))
	{
		typedef typename ItemTraits::template Creator<std::iter_reference_t<ArgIterator>> IterCreator;
		MemManager& thisMemManager = GetMemManager();
		if constexpr (internal::conceptForwardIterator<ArgIterator>)
			pvInitCapacity(internal::UIntMath<>::Dist(begin, end));
		try
		{
			for (ArgIterator iter = begin; iter != end; ++iter)
			{
				if constexpr (internal::conceptForwardIterator<ArgIterator>)
					AddBackNogrowCrt(IterCreator(thisMemManager, *iter));
				else
					AddBackCrt(IterCreator(thisMemManager, *iter));
			}
		}
		catch (...)
		{
			pvDestroy();
			throw;
		}
	}

	MergeArray(std::initializer_list<Item> items)
		: MergeArray(items.begin(), items.end())
	{
	}

	explicit MergeArray(std::initializer_list<Item> items, MemManager memManager)
		: MergeArray(items.begin(), items.end(), std::move(memManager))
	{
	}

	MergeArray(MergeArray&& array) noexcept
		: mSegments(std::move(array.mSegments)),
		mCount(array.mCount),
		mCapacity(array.mCapacity)
	{
		array.mCount = 0;
		array.mCapacity = 0;
	}

	MergeArray(const MergeArray& array)
		: MergeArray(array, true)
	{
	}

	explicit MergeArray(const MergeArray& array, bool shrink)
		: MergeArray(MemManager(array.GetMemManager()))
	{
		pvInitCapacity(shrink ? array.GetCount() : array.GetCapacity());
		try
		{
			for (const Item& item : array)
				AddBackNogrow(item);
		}
		catch (...)
		{
			pvDestroy();
			throw;
		}
	}

	explicit MergeArray(const MergeArray& array, MemManager memManager)
		: MergeArray(array.GetBegin(), array.GetEnd(), std::move(memManager))
	{
	}

	static MergeArray CreateCap(size_t capacity, MemManager memManager = MemManager())
	{
		MergeArray array(std::move(memManager));
		array.pvInitCapacity(capacity);
		return array;
	}

	template<typename MultiItemCreator>
	requires std::regular_invocable<const MultiItemCreator&, Item*>
	static MergeArray CreateCrt(size_t count, const MultiItemCreator& multiItemCreator,
		MemManager memManager = MemManager())
	{
		MergeArray array = CreateCap(count, std::move(memManager));
		for (size_t i = 0; i < count; ++i)
			array.AddBackNogrowCrt(multiItemCreator);
		return array;
	}

	~MergeArray() noexcept
	{
		pvDestroy();
	}

	MergeArray& operator=(MergeArray&& array) noexcept
	{
		MergeArray(std::move(array)).Swap(*this);
		return *this;
	}

	MergeArray& operator=(const MergeArray& array)
	{
		if (this != &array)
			MergeArray(array).Swap(*this);
		return *this;
	}

	void Swap(MergeArray& array) noexcept
	{
		mSegments.Swap(array.mSegments);
		std::swap(mCount, array.mCount);
		std::swap(mCapacity, array.mCapacity);
	}

	ConstIterator GetBegin() const noexcept
	{
		return ConstIteratorProxy(this, size_t{0});
	}

	Iterator GetBegin() noexcept
	{
		return pvMakeIterator(0);
	}

	ConstIterator GetEnd() const noexcept
	{
		return ConstIteratorProxy(this, mCount);
	}

	Iterator GetEnd() noexcept
	{
		return pvMakeIterator(mCount);
	}

	MOMO_FRIEND_SWAP(MergeArray)
	MOMO_FRIENDS_SIZE_BEGIN_END(MergeArray)

	const MemManager& GetMemManager() const noexcept
	{
		return mSegments.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mSegments.GetMemManager();
	}

	size_t GetCount() const noexcept
	{
		return mCount;
	}

	template<typename MultiItemCreator>
	requires std::regular_invocable<const MultiItemCreator&, Item*>
	void SetCountCrt(size_t count, const MultiItemCreator& multiItemCreator)
	{
		if (count <= mCount)
		{
			pvRemoveBack(mCount - count);
		}
		else //if (count <= mCapacity)
		{
			Reserve(count);	//?
			size_t initCount = mCount;
			try
			{
				for (size_t i = initCount; i < count; ++i)
					AddBackNogrowCrt(multiItemCreator);
			}
			catch (...)
			{
				pvRemoveBack(mCount - initCount);
				throw;
			}
		}
	}

	void SetCount(size_t count)
	{
		typedef typename ItemTraits::template Creator<> Creator;
		MemManager& memManager = GetMemManager();
		auto multiItemCreator = [&memManager] (Item* newItem)
			{ (Creator(memManager))(newItem); };
		SetCountCrt(count, multiItemCreator);
	}

	void SetCount(size_t count, const Item& item)
	{
		typedef typename ItemTraits::template Creator<const Item&> Creator;
		MemManager& memManager = GetMemManager();
		auto multiItemCreator = [&memManager, &item] (Item* newItem)
			{ Creator(memManager, item)(newItem); };
		SetCountCrt(count, multiItemCreator);
	}

	bool IsEmpty() const noexcept
	{
		return mCount == 0;
	}

	void Clear(bool shrink = false) noexcept
	{
		pvRemoveBack(mCount);
		if (shrink)
		{
			mCapacity = 0;
			pvDeallocateSegments();
			mSegments.Clear(true);
		}
	}

	size_t GetCapacity() const noexcept
	{
		return mCapacity;
	}

	void Reserve(size_t capacity)
	{
		if (capacity <= mCapacity)
			return;
		if (mCapacity == 0)
			return pvInitCapacity(capacity);
		capacity = pvCeilCapacity(capacity);
		size_t segIndex = pvGetSegIndex(mCapacity - 1, capacity);
		if (mSegments.GetCount() <= segIndex)
			mSegments.SetCount(segIndex + 1, nullptr);
		mSegments[segIndex] = pvAllocateSegment(segIndex);
		try
		{
			for (size_t i = 1; i < segIndex; ++i)
			{
				if (mSegments[i] == nullptr && pvHasSegment(i, capacity))
					mSegments[i] = pvAllocateSegment(i);
			}
			if (mCount >> (segIndex + logInitialItemCount) ==
				mCapacity >> (segIndex + logInitialItemCount))
			{
				size_t segItemCount = mCount & ((initialItemCount << segIndex) - 1);
				ItemTraits::Relocate(GetMemManager(), pvMakeIterator(mCount - segItemCount),
					mSegments[segIndex], segItemCount);
			}
		}
		catch (...)
		{
			pvDeallocateSegments();
			throw;
		}
		mCapacity = capacity;
		pvDeallocateSegments();
	}

	void Shrink()
	{
		Shrink(mCount);
	}

	void Shrink(size_t capacity)
	{
		if (capacity < mCount)
			capacity = mCount;
		if (capacity == 0)
			return Clear(true);
		capacity = pvCeilCapacity(capacity);
		if (capacity >= mCapacity)
			return;
		size_t initCapacity = mCapacity;
		mCapacity = capacity;
		size_t segIndex = pvGetSegIndex(mCapacity - 1, initCapacity);
		try
		{
			for (size_t i = 1; i < segIndex; ++i)
			{
				if (mSegments[i] == nullptr && pvHasSegment(i, mCapacity))
					mSegments[i] = pvAllocateSegment(i);
			}
			if (mCount >> (segIndex + logInitialItemCount) ==
				mCapacity >> (segIndex + logInitialItemCount))
			{
				size_t segItemCount = mCount & ((initialItemCount << segIndex) - 1);
				ItemTraits::Relocate(GetMemManager(), mSegments[segIndex],
					pvMakeIterator(mCount - segItemCount), segItemCount);
			}
		}
		catch (...)
		{
			mCapacity = initCapacity;
			pvDeallocateSegments();
			throw;
		}
		pvDeallocateSegments();
	}

	const Item& operator[](size_t index) const
	{
		return pvGetItem(index);
	}

	Item& operator[](size_t index)
	{
		return pvGetItem(index);
	}

	const Item& GetBackItem() const
	{
		return pvGetItem(mCount - 1);
	}

	Item& GetBackItem()
	{
		return pvGetItem(mCount - 1);
	}

	template<typename ItemCreator>
	requires std::invocable<ItemCreator&&, Item*>
	void AddBackNogrowCrt(ItemCreator&& itemCreator)
	{
		MOMO_CHECK(mCount < mCapacity);
		std::forward<ItemCreator>(itemCreator)(pvGetItemPtr(mCount));
		++mCount;
	}

	template<typename... ItemArgs>
	void AddBackNogrowVar(ItemArgs&&... itemArgs)
	{
		AddBackNogrowCrt(typename ItemTraits::template Creator<ItemArgs...>(GetMemManager(),
			std::forward<ItemArgs>(itemArgs)...));
	}

	void AddBackNogrow(Item&& item)
	{
		AddBackNogrowVar(std::move(item));
	}

	void AddBackNogrow(const Item& item)
	{
		AddBackNogrowVar(item);
	}

	template<typename ItemCreator>
	requires std::invocable<ItemCreator&&, Item*>
	void AddBackCrt(ItemCreator&& itemCreator)
	{
		if (mCount < mCapacity)
		{
			std::forward<ItemCreator>(itemCreator)(pvGetItemPtr(mCount));
		}
		else if ((mCount & initialItemCount) > 0 || mCount == 0)
		{
			if (mSegments.GetCount() <= 1)
				mSegments.SetCount(2, nullptr);
			Item* segment0 = pvAllocateSegment(0);
			try
			{
				std::forward<ItemCreator>(itemCreator)(segment0);
			}
			catch (...)
			{
				pvDeallocateSegment(0, segment0);
				throw;
			}
			mCapacity += initialItemCount;
			mSegments[1] = mSegments[0];
			mSegments[0] = segment0;
		}
		else
		{
			size_t segIndex = static_cast<size_t>(std::countr_zero(mCount >> logInitialItemCount) + 1);
			if (mSegments.GetCount() <= segIndex)
				mSegments.SetCount(segIndex + 1, nullptr);
			Item* segment0 = pvAllocateSegment(0);
			try
			{
				mSegments[segIndex] = pvAllocateSegment(segIndex);
				size_t segItemCount = pvGetSegItemCount(segIndex);
				ItemTraits::RelocateCreate(GetMemManager(), pvMakeIterator(mCount - segItemCount),
					mSegments[segIndex], segItemCount, std::forward<ItemCreator>(itemCreator), segment0);
			}
			catch (...)
			{
				pvDeallocateSegments();
				pvDeallocateSegment(0, segment0);
				throw;
			}
			mCapacity += initialItemCount;
			pvDeallocateSegments();
			pvDeallocateSegment(0, mSegments[0]);
			mSegments[0] = segment0;
		}
		++mCount;
	}

	template<typename... ItemArgs>
	void AddBackVar(ItemArgs&&... itemArgs)
	{
		AddBackCrt(typename ItemTraits::template Creator<ItemArgs...>(GetMemManager(),
			std::forward<ItemArgs>(itemArgs)...));
	}

	void AddBack(Item&& item)
	{
		AddBackVar(std::move(item));
	}

	void AddBack(const Item& item)
	{
		AddBackVar(item);
	}

	template<typename ItemCreator>
	requires std::invocable<ItemCreator&&, Item*>
	void InsertCrt(size_t index, ItemCreator&& itemCreator)
	{
		ItemHandler itemHandler(GetMemManager(), std::forward<ItemCreator>(itemCreator));
		std::move_iterator<Item*> begin(&itemHandler);
		Insert(index, begin, begin + 1);
	}

	template<typename... ItemArgs>
	void InsertVar(size_t index, ItemArgs&&... itemArgs)
	{
		InsertCrt(index, typename ItemTraits::template Creator<ItemArgs...>(GetMemManager(),
			std::forward<ItemArgs>(itemArgs)...));
	}

	void Insert(size_t index, Item&& item)
	{
		InsertVar(index, std::move(item));
	}

	void Insert(size_t index, const Item& item)
	{
		InsertVar(index, item);
	}

	void Insert(size_t index, size_t count, const Item& item)
	{
		MemManager& memManager = GetMemManager();
		ItemHandler itemHandler(memManager,
			typename ItemTraits::template Creator<const Item&>(memManager, item));
		Reserve(mCount + count);
		ArrayShifter::Insert(*this, index, count, *&itemHandler);
	}

	template<internal::conceptInputIterator ArgIterator>
	void Insert(size_t index, ArgIterator begin, ArgIterator end)
	{
		if constexpr (internal::conceptForwardIterator<ArgIterator>)
			Reserve(mCount + internal::UIntMath<>::Dist(begin, end));
		ArrayShifter::Insert(*this, index, begin, end);
	}

	void Insert(size_t index, std::initializer_list<Item> items)
	{
		Insert(index, items.begin(), items.end());
	}

	void RemoveBack(size_t count = 1)
	{
		MOMO_CHECK(count <= mCount);
		pvRemoveBack(count);
	}

	void Remove(size_t index, size_t count = 1)
	{
		ArrayShifter::Remove(*this, index, count);
	}

	template<typename Predicate>
	requires std::predicate<const Predicate&, const Item&>
	size_t Remove(const Predicate& pred)
	{
		return ArrayShifter::Remove(*this, pred);
	}

	template<typename ItemArg,
		typename EqualFunc = std::equal_to<>>
	requires std::equivalence_relation<const EqualFunc&, const ItemArg&, const Item&>
	bool Contains(const ItemArg& itemArg, const EqualFunc& equalFunc = EqualFunc()) const
	{
		ConstIterator end = GetEnd();
		return std::find_if(GetBegin(), end,
			[&itemArg, &equalFunc] (const Item& item) { return equalFunc(itemArg, item); }) != end;
	}

	template<typename EqualFunc = std::equal_to<Item>>
	requires std::equivalence_relation<const EqualFunc&, const Item&, const Item&>
	bool IsEqual(const MergeArray& array, const EqualFunc& equalFunc = EqualFunc()) const
	{
		return GetCount() == array.GetCount() &&
			std::equal(GetBegin(), GetEnd(), array.GetBegin(), equalFunc);
	}

private:
	static size_t pvGetSegItemCount(size_t segIndex) noexcept
	{
		return initialItemCount << ((segIndex > 0) ? segIndex - 1 : 0);
	}

	static bool pvHasSegment(size_t segIndex, size_t capacity) noexcept
	{
		if (capacity == 0)
			return false;
		if (segIndex == 0)
			return true;
		return ((capacity - 1) & (initialItemCount << (segIndex - 1))) > 0;
	}

	static size_t pvGetSegIndex(size_t index, size_t capacity) noexcept
	{
		MOMO_ASSERT(index < capacity);
		return std::bit_width((index ^ (capacity - 1)) >> logInitialItemCount);
	}

	static size_t pvCeilCapacity(size_t capacity) noexcept
	{
		return internal::UIntMath<>::Ceil(capacity, initialItemCount);
	}

	Item* pvAllocateSegment(size_t segIndex)
	{
		size_t segItemCount = pvGetSegItemCount(segIndex);
		if (segItemCount > internal::UIntConst::maxSize / sizeof(Item))
			throw std::bad_array_new_length();
		static_assert(internal::ObjectAlignmenter<Item>::Check(ItemTraits::alignment));
		return MemManagerProxy::template Allocate<Item>(GetMemManager(),
			segItemCount * sizeof(Item));
	}

	void pvDeallocateSegment(size_t segIndex, Item* segment) noexcept
	{
		size_t segItemCount = pvGetSegItemCount(segIndex);
		MemManagerProxy::Deallocate(GetMemManager(), segment, segItemCount * sizeof(Item));
	}

	void pvDeallocateSegments() noexcept
	{
		size_t segCount = mSegments.GetCount();
		for (size_t i = 0; i < segCount; ++i)
		{
			Item*& segment = mSegments[i];
			bool hasSegment = pvHasSegment(i, mCapacity);
			MOMO_ASSERT(!(segment == nullptr && hasSegment));
			if (segment != nullptr && !hasSegment)
			{
				pvDeallocateSegment(i, segment);
				segment = nullptr;
			}
		}
	}

	void pvDestroy() noexcept
	{
		pvRemoveBack(mCount);
		mCapacity = 0;
		pvDeallocateSegments();
	}

	void pvInitCapacity(size_t capacity)
	{
		MOMO_ASSERT(mCapacity == 0);
		if (capacity == 0)
			return;
		capacity = pvCeilCapacity(capacity);
		size_t segCount = pvGetSegIndex(0, capacity) + 1;
		mSegments.SetCount(segCount, nullptr);
		try
		{
			for (size_t i = 0; i < segCount; ++i)
			{
				if (pvHasSegment(i, capacity))
					mSegments[i] = pvAllocateSegment(i);
			}
		}
		catch (...)
		{
			pvDeallocateSegments();
			throw;
		}
		mCapacity = capacity;
	}

	Iterator pvMakeIterator(size_t index) noexcept
	{
		return IteratorProxy(this, index);
	}

	Item& pvGetItem(size_t index) const
	{
		MOMO_CHECK(index < mCount);
		return *pvGetItemPtr(index);
	}

	Item* pvGetItemPtr(size_t index) const noexcept
	{
		size_t segIndex = pvGetSegIndex(index, mCapacity);
		size_t segItemIndex = index & ((initialItemCount << segIndex) - 1);
		return mSegments[segIndex] + segItemIndex;
	}

	void pvRemoveBack(size_t count) noexcept
	{
		ItemTraits::Destroy(GetMemManager(), pvMakeIterator(mCount - count), count);
		mCount -= count;
	}

private:
	Segments mSegments;
	size_t mCount;
	size_t mCapacity;
};

} // namespace momo

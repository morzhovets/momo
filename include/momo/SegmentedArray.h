/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/SegmentedArray.h

  namespace momo:
    class SegmentedArrayItemTraits
    enum class SegmentedArrayItemCountFunc
    class SegmentedArraySettings
    class SegmentedArray
    class SegmentedArraySqrt

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_SEGMENTED_ARRAY
#define MOMO_INCLUDE_GUARD_SEGMENTED_ARRAY

#include "Array.h"

namespace momo
{

template<typename TItem, typename TMemManager>
class SegmentedArrayItemTraits
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
	static void Destroy(MemManager& memManager, Item* items, size_t count) noexcept
	{
		ItemManager::Destroy(memManager, items, count);
	}

	template<typename ItemArg>
	static void Assign(MemManager& /*memManager*/, ItemArg&& itemArg, Item& item)
	{
		item = std::forward<ItemArg>(itemArg);
	}
};

enum class SegmentedArrayItemCountFunc
{
	sqrt = 0,
	cnst = 1,
};

template<SegmentedArrayItemCountFunc tItemCountFunc = SegmentedArrayItemCountFunc::cnst,
	size_t tLogInitialItemCount =
		(tItemCountFunc == SegmentedArrayItemCountFunc::cnst) ? 5 : 3>
class SegmentedArraySettings;

template<size_t tLogInitialItemCount>
class SegmentedArraySettings<SegmentedArrayItemCountFunc::sqrt, tLogInitialItemCount>
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;

	static const SegmentedArrayItemCountFunc itemCountFunc = SegmentedArrayItemCountFunc::sqrt;
	static const size_t logInitialItemCount = tLogInitialItemCount;

	typedef ArraySettings<> SegmentsSettings;

public:
	static void GetSegItemIndexes(size_t index, size_t& segIndex, size_t& itemIndex) noexcept
	{
		size_t index1 = (index >> logInitialItemCount) + 1;
		size_t index2 = index & ((size_t{1} << logInitialItemCount) - 1);
		size_t logItemCount = pvIndexToLogItemCount(index1);
		size_t itemIndex1 = index1 & ((size_t{1} << logItemCount) - 1);
		size_t itemIndex2 = index2;
		segIndex = (index1 >> logItemCount) + (size_t{1} << logItemCount) - 2;
		itemIndex = (itemIndex1 << logInitialItemCount) + itemIndex2;
	}

	static size_t GetIndex(size_t segIndex, size_t itemIndex) noexcept
	{
		size_t itemIndex1 = itemIndex >> logInitialItemCount;
		size_t itemIndex2 = itemIndex & ((size_t{1} << logInitialItemCount) - 1);
		size_t logItemCount = pvSegIndexToLogItemCount(segIndex);
		size_t index1 = ((segIndex + 2 - (size_t{1} << logItemCount)) << logItemCount) + itemIndex1;
		size_t index2 = itemIndex2;
		size_t index = ((index1 - 1) << logInitialItemCount) + index2;
		return index;
	}

	static size_t GetItemCount(size_t segIndex) noexcept
	{
		size_t logItemCount = pvSegIndexToLogItemCount(segIndex);
		return size_t{1} << (logItemCount + logInitialItemCount);
	}

private:
	static size_t pvIndexToLogItemCount(size_t index1) noexcept
	{
		return (internal::UIntMath<>::Log2(index1) + 1) / 2;
	}

	static size_t pvSegIndexToLogItemCount(size_t segIndex) noexcept
	{
		return internal::UIntMath<>::Log2((segIndex * 2 + 4) / 3);
	}
};

template<size_t tLogInitialItemCount>
class SegmentedArraySettings<SegmentedArrayItemCountFunc::cnst, tLogInitialItemCount>
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;

	static const SegmentedArrayItemCountFunc itemCountFunc = SegmentedArrayItemCountFunc::cnst;
	static const size_t logInitialItemCount = tLogInitialItemCount;

	typedef ArraySettings<> SegmentsSettings;

public:
	static void GetSegItemIndexes(size_t index, size_t& segIndex, size_t& itemIndex) noexcept
	{
		segIndex = index >> logInitialItemCount;
		itemIndex = index & ((size_t{1} << logInitialItemCount) - 1);
	}

	static size_t GetIndex(size_t segIndex, size_t itemIndex) noexcept
	{
		return (segIndex << logInitialItemCount) + itemIndex;
	}

	static size_t GetItemCount(size_t /*segIndex*/) noexcept
	{
		return size_t{1} << logInitialItemCount;
	}
};

/*!
	All `SegmentedArray` functions and constructors have strong exception safety,
	but not the following cases:
	 - Functions `Insert`, `InsertVar`, `InsertCrt`, `Remove` have
	basic exception safety.

	Swap and move operations invalidate all container iterators.
*/

template<typename TItem,
	typename TMemManager = MemManagerDefault,
	typename TItemTraits = SegmentedArrayItemTraits<TItem, TMemManager>,
	typename TSettings = SegmentedArraySettings<>>
class SegmentedArray
{
public:
	typedef TItem Item;
	typedef TMemManager MemManager;
	typedef TItemTraits ItemTraits;
	typedef TSettings Settings;

	typedef internal::ArrayIndexIterator<SegmentedArray, Item> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

private:
	typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

	typedef internal::NestedArraySettings<typename Settings::SegmentsSettings> SegmentsSettings;

	typedef Array<Item*, MemManager, ArrayItemTraits<Item*, MemManager>,
		SegmentsSettings> Segments;

	typedef internal::ArrayItemHandler<ItemTraits> ItemHandler;
	typedef internal::ArrayShifter<SegmentedArray> ArrayShifter;

	struct ConstIteratorProxy : public ConstIterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
	};

	struct IteratorProxy : public Iterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
	};

public:
	SegmentedArray() noexcept(std::is_nothrow_default_constructible<MemManager>::value)	// vs2017
		: SegmentedArray(MemManager())
	{
	}

	explicit SegmentedArray(MemManager memManager) noexcept
		: mSegments(std::move(memManager)),
		mCount(0)
	{
	}

	explicit SegmentedArray(size_t count, MemManager memManager = MemManager())
		: SegmentedArray(std::move(memManager))
	{
		SetCount(count);
	}

	explicit SegmentedArray(size_t count, const Item& item, MemManager memManager = MemManager())
		: SegmentedArray(std::move(memManager))
	{
		SetCount(count, item);
	}

	template<typename ArgIterator, typename ArgSentinel,
		typename = decltype(*std::declval<ArgIterator>())>
	explicit SegmentedArray(ArgIterator begin, ArgSentinel end, MemManager memManager = MemManager())
		: SegmentedArray(std::move(memManager))
	{
		try
		{
			typedef typename ItemTraits::template Creator<decltype(*begin)> IterCreator;
			for (ArgIterator iter = std::move(begin); iter != end; ++iter)
				AddBackCrt(IterCreator(GetMemManager(), *iter));
		}
		catch (...)
		{
			pvDecCount(0);
			pvDecCapacity(0);
			throw;
		}
	}

	SegmentedArray(std::initializer_list<Item> items)
		: SegmentedArray(items, MemManager())
	{
	}

	explicit SegmentedArray(std::initializer_list<Item> items, MemManager memManager)
		: SegmentedArray(items.begin(), items.end(), std::move(memManager))
	{
	}

	SegmentedArray(SegmentedArray&& array) noexcept
		: mSegments(std::move(array.mSegments)),
		mCount(array.mCount)
	{
		array.mCount = 0;
	}

	SegmentedArray(const SegmentedArray& array)
		: SegmentedArray(array, true)
	{
	}

	explicit SegmentedArray(const SegmentedArray& array, bool shrink)
		: SegmentedArray(MemManager(array.GetMemManager()))
	{
		pvIncCapacity(0, shrink ? array.GetCount() : array.GetCapacity());
		try
		{
			for (const Item& item : array)
				AddBackNogrow(item);
		}
		catch (...)
		{
			pvDecCount(0);
			pvDecCapacity(0);
			throw;
		}
	}

	explicit SegmentedArray(const SegmentedArray& array, MemManager memManager)
		: SegmentedArray(array.GetBegin(), array.GetEnd(), std::move(memManager))
	{
	}

	static SegmentedArray CreateCap(size_t capacity, MemManager memManager = MemManager())
	{
		SegmentedArray array(std::move(memManager));
		array.pvIncCapacity(0, capacity);
		return array;
	}

	template<typename ItemMultiCreator>
	static SegmentedArray CreateCrt(size_t count, const ItemMultiCreator& itemMultiCreator,
		MemManager memManager = MemManager())
	{
		SegmentedArray array = CreateCap(count, std::move(memManager));
		array.pvIncCount(count, itemMultiCreator);
		return array;
	}

	~SegmentedArray() noexcept
	{
		pvDecCount(0);
		pvDecCapacity(0);
	}

	SegmentedArray& operator=(SegmentedArray&& array) noexcept
	{
		return internal::ContainerAssigner::Move(std::move(array), *this);
	}

	SegmentedArray& operator=(const SegmentedArray& array)
	{
		return internal::ContainerAssigner::Copy(array, *this);
	}

	void Swap(SegmentedArray& array) noexcept
	{
		mSegments.Swap(array.mSegments);
		std::swap(mCount, array.mCount);
	}

	ConstIterator GetBegin() const noexcept
	{
		return ConstIteratorProxy(this, size_t{0});
	}

	Iterator GetBegin() noexcept
	{
		return IteratorProxy(this, size_t{0});
	}

	ConstIterator GetEnd() const noexcept
	{
		return ConstIteratorProxy(this, mCount);
	}

	Iterator GetEnd() noexcept
	{
		return IteratorProxy(this, mCount);
	}

	MOMO_FRIEND_SWAP(SegmentedArray)
	MOMO_FRIENDS_SIZE_BEGIN_END_CONST(SegmentedArray, ConstIterator)
	MOMO_FRIENDS_BEGIN_END(SegmentedArray, Iterator)

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

	template<typename ItemMultiCreator>
	void SetCountCrt(size_t count, const ItemMultiCreator& itemMultiCreator)
	{
		if (count < mCount)
			pvDecCount(count);
		else if (count > mCount)
			pvIncCount(count, itemMultiCreator);
	}

	void SetCount(size_t count)
	{
		typedef typename ItemTraits::template Creator<> Creator;
		MemManager& memManager = GetMemManager();
		auto itemMultiCreator = [&memManager] (Item* newItem)
			{ (Creator(memManager))(newItem); };
		SetCountCrt(count, itemMultiCreator);
	}

	void SetCount(size_t count, const Item& item)
	{
		typedef typename ItemTraits::template Creator<const Item&> Creator;
		MemManager& memManager = GetMemManager();
		auto itemMultiCreator = [&memManager, &item] (Item* newItem)
			{ Creator(memManager, item)(newItem); };
		SetCountCrt(count, itemMultiCreator);
	}

	bool IsEmpty() const noexcept
	{
		return mCount == 0;
	}

	void Clear(bool shrink = false) noexcept
	{
		pvDecCount(0);
		if (shrink)
		{
			pvDecCapacity(0);
			mSegments.Clear(true);
		}
	}

	size_t GetCapacity() const noexcept
	{
		return Settings::GetIndex(mSegments.GetCount(), 0);	//?
	}

	void Reserve(size_t capacity)
	{
		size_t initCapacity = GetCapacity();
		if (capacity > initCapacity)
			pvIncCapacity(initCapacity, capacity);
	}

	void Shrink() noexcept
	{
		Shrink(mCount);
	}

	void Shrink(size_t capacity) noexcept
	{
		if (GetCapacity() <= capacity)
			return;
		if (capacity < mCount)
			capacity = mCount;
		pvDecCapacity(capacity);
		try
		{
			mSegments.Shrink();
		}
		catch (...)
		{
			// no throw!
		}
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
	void AddBackNogrowCrt(ItemCreator&& itemCreator)
	{
		size_t segIndex, itemIndex;
		Settings::GetSegItemIndexes(mCount, segIndex, itemIndex);
		MOMO_CHECK(segIndex < mSegments.GetCount());
		std::forward<ItemCreator>(itemCreator)(mSegments[segIndex] + itemIndex);
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
	void AddBackCrt(ItemCreator&& itemCreator)
	{
		size_t segIndex, itemIndex;
		Settings::GetSegItemIndexes(mCount, segIndex, itemIndex);
		size_t segCount = mSegments.GetCount();
		if (segIndex < segCount)
		{
			std::forward<ItemCreator>(itemCreator)(mSegments[segIndex] + itemIndex);
		}
		else
		{
			MOMO_ASSERT(itemIndex == 0);
			mSegments.Reserve(segCount + 1);
			Item* segment = pvAllocateSegment(segCount);
			try
			{
				std::forward<ItemCreator>(itemCreator)(segment);
			}
			catch (...)
			{
				pvDeallocateSegment(segCount, segment);
				throw;
			}
			mSegments.AddBackNogrow(segment);
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
	void InsertCrt(size_t index, ItemCreator&& itemCreator)
	{
		ItemHandler itemHandler(GetMemManager(), std::forward<ItemCreator>(itemCreator));
		Reserve(mCount + 1);
		ArrayShifter::InsertNogrow(*this, index, std::move(itemHandler.Get()));
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
		ArrayShifter::InsertNogrow(*this, index, count, itemHandler.Get());
	}

	template<typename ArgIterator, typename ArgSentinel,
		typename = decltype(*std::declval<ArgIterator>())>
	void Insert(size_t index, ArgIterator begin, ArgSentinel end)
	{
		pvInsert(index, std::move(begin), std::move(end));
	}

	void Insert(size_t index, std::initializer_list<Item> items)
	{
		pvInsert(index, items.begin(), items.end());
	}

	void RemoveBack(size_t count = 1)
	{
		MOMO_CHECK(count <= mCount);
		pvDecCount(mCount - count);
	}

	void Remove(size_t index, size_t count = 1)
	{
		ArrayShifter::Remove(*this, index, count);
	}

	template<typename ItemFilter>
	internal::EnableIf<internal::IsInvocable<const ItemFilter&, bool, const Item&>::value,
	size_t> Remove(const ItemFilter& itemFilter)
	{
		return ArrayShifter::Remove(*this, itemFilter);
	}

	template<typename ItemEqualComparer = std::equal_to<Item>>
	bool Contains(const Item& item,
		const ItemEqualComparer& itemEqualComp = ItemEqualComparer()) const
	{
		return std::any_of(GetBegin(), GetEnd(),
			[&item, &itemEqualComp] (const Item& thisItem) { return itemEqualComp(thisItem, item); });
	}

	template<typename ItemEqualComparer = std::equal_to<Item>>
	bool IsEqual(const SegmentedArray& array,
		const ItemEqualComparer& itemEqualComp = ItemEqualComparer()) const
	{
		return GetCount() == array.GetCount() &&
			std::equal(GetBegin(), GetEnd(), array.GetBegin(), itemEqualComp);
	}

private:
	Item* pvAllocateSegment(size_t segIndex)
	{
		size_t itemCount = Settings::GetItemCount(segIndex);
		if (itemCount > internal::UIntConst::maxSize / sizeof(Item))
			MOMO_THROW(std::length_error("Invalid item count"));
		MOMO_STATIC_ASSERT(internal::ObjectAlignmenter<Item>::Check(ItemTraits::alignment));
		return MemManagerProxy::template Allocate<Item>(GetMemManager(),
			itemCount * sizeof(Item));
	}

	void pvDeallocateSegment(size_t segIndex, Item* segment) noexcept
	{
		size_t itemCount = Settings::GetItemCount(segIndex);
		MemManagerProxy::Deallocate(GetMemManager(), segment, itemCount * sizeof(Item));
	}

	Item& pvGetItem(size_t index) const
	{
		MOMO_CHECK(index < mCount);
		size_t segIndex, itemIndex;
		Settings::GetSegItemIndexes(index, segIndex, itemIndex);
		return mSegments[segIndex][itemIndex];
	}

	template<typename ItemMultiCreator>
	void pvIncCount(size_t count, const ItemMultiCreator& itemMultiCreator)
	{
		MOMO_ASSERT(count >= mCount);
		size_t initCapacity = GetCapacity();
		size_t initCount = mCount;
		if (count > initCapacity)
			pvIncCapacity(initCapacity, count);
		try
		{
			size_t segIndex, itemIndex;
			Settings::GetSegItemIndexes(mCount, segIndex, itemIndex);
			while (mCount < count)
			{
				Item* segment = mSegments[segIndex];
				size_t itemCount = Settings::GetItemCount(segIndex);
				for (; itemIndex < itemCount && mCount < count; ++itemIndex, ++mCount)
					itemMultiCreator(segment + itemIndex);
				if (itemIndex == itemCount)
				{
					++segIndex;
					itemIndex = 0;
				}
			}
		}
		catch (...)
		{
			pvDecCount(initCount);
			pvDecCapacity(initCapacity);
			throw;
		}
	}

	void pvDecCount(size_t count) noexcept
	{
		MOMO_ASSERT(count <= mCount);
		size_t segIndex, itemIndex;
		Settings::GetSegItemIndexes(mCount, segIndex, itemIndex);
		MemManager& memManager = GetMemManager();
		while (mCount > count)
		{
			if (itemIndex == 0)
			{
				--segIndex;
				itemIndex = Settings::GetItemCount(segIndex);
			}
			size_t remCount = std::minmax(itemIndex, mCount - count).first;
			ItemTraits::Destroy(memManager, mSegments[segIndex] + itemIndex - remCount, remCount);
			itemIndex -= remCount;
			mCount -= remCount;
		}
	}

	void pvIncCapacity(size_t initCapacity, size_t capacity)
	{
		MOMO_ASSERT(capacity >= initCapacity);
		size_t segIndex, itemIndex;
		Settings::GetSegItemIndexes(capacity, segIndex, itemIndex);
		if (itemIndex > 0)
			++segIndex;
		try
		{
			for (size_t segCount = mSegments.GetCount(); segCount < segIndex; ++segCount)
			{
				mSegments.Reserve(segCount + 1);
				Item* segment = pvAllocateSegment(segCount);
				mSegments.AddBackNogrow(segment);
			}
		}
		catch (...)
		{
			pvDecCapacity(initCapacity);
			throw;
		}
	}

	void pvDecCapacity(size_t capacity) noexcept
	{
		MOMO_ASSERT(capacity <= GetCapacity());
		size_t segIndex, itemIndex;
		Settings::GetSegItemIndexes(capacity, segIndex, itemIndex);
		if (itemIndex > 0)
			++segIndex;
		size_t segCount = mSegments.GetCount();
		for (size_t i = segIndex; i < segCount; ++i)
			pvDeallocateSegment(i, mSegments[i]);
		mSegments.RemoveBack(segCount - segIndex);
	}

	template<typename ArgIterator, typename ArgSentinel>
	internal::EnableIf<internal::IsForwardIterator17<ArgIterator, ArgSentinel>::value>
	pvInsert(size_t index, ArgIterator begin, ArgSentinel end)
	{
		size_t count = internal::UIntMath<>::Dist(begin, end);
		Reserve(mCount + count);
		ArrayShifter::InsertNogrow(*this, index, begin, count);
	}

	template<typename ArgIterator, typename ArgSentinel>
	internal::EnableIf<!internal::IsForwardIterator17<ArgIterator, ArgSentinel>::value>
	pvInsert(size_t index, ArgIterator begin, ArgSentinel end)
	{
		ArrayShifter::Insert(*this, index, std::move(begin), std::move(end));
	}

private:
	Segments mSegments;
	size_t mCount;
};

template<typename TItem,
	typename TMemManager = MemManagerDefault,
	typename TItemTraits = SegmentedArrayItemTraits<TItem, TMemManager>>
using SegmentedArraySqrt = SegmentedArray<TItem, TMemManager, TItemTraits,
	SegmentedArraySettings<SegmentedArrayItemCountFunc::sqrt>>;

} // namespace momo

namespace std
{
	template<typename... Params>
	class back_insert_iterator<momo::SegmentedArray<Params...>>
		: public momo::internal::BackInsertIteratorStdBase<momo::SegmentedArray<Params...>>
	{
	private:
		typedef momo::internal::BackInsertIteratorStdBase<momo::SegmentedArray<Params...>>
			BackInsertIteratorStdBase;

	public:
		using BackInsertIteratorStdBase::BackInsertIteratorStdBase;
		using BackInsertIteratorStdBase::operator=;
	};
} // namespace std

#endif // MOMO_INCLUDE_GUARD_SEGMENTED_ARRAY

/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/SegmentedArray.h

  namespace momo:
    class SegmentedArrayItemTraits
    enum class SegmentedArrayItemCountFunc
    class SegmentedArraySettings
    class SegmentedArray
    class SegmentedArraySqrt

\**********************************************************/

#pragma once

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
	SegmentedArray() noexcept(noexcept(MemManager()))
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

	template<typename ArgIterator,
		typename = typename std::iterator_traits<ArgIterator>::iterator_category>
	explicit SegmentedArray(ArgIterator begin, ArgIterator end, MemManager memManager = MemManager())
		: SegmentedArray(std::move(memManager))
	{
		try
		{
			typedef typename ItemTraits::template Creator<
				typename std::iterator_traits<ArgIterator>::reference> IterCreator;
			for (ArgIterator iter = begin; iter != end; ++iter)
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
		: SegmentedArray(items.begin(), items.end())
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

	template<typename MultiItemCreator>
	static SegmentedArray CreateCrt(size_t count, const MultiItemCreator& multiItemCreator,
		MemManager memManager = MemManager())
	{
		SegmentedArray array = CreateCap(count, std::move(memManager));
		array.pvIncCount(count, multiItemCreator);
		return array;
	}

	~SegmentedArray() noexcept
	{
		pvDecCount(0);
		pvDecCapacity(0);
	}

	SegmentedArray& operator=(SegmentedArray&& array) noexcept
	{
		SegmentedArray(std::move(array)).Swap(*this);
		return *this;
	}

	SegmentedArray& operator=(const SegmentedArray& array)
	{
		if (this != &array)
			SegmentedArray(array).Swap(*this);
		return *this;
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
	MOMO_FRIENDS_SIZE_BEGIN_END_CONST(SegmentedArray)
	MOMO_FRIENDS_BEGIN_END(SegmentedArray)

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
	void SetCountCrt(size_t count, const MultiItemCreator& multiItemCreator)
	{
		if (count < mCount)
			pvDecCount(count);
		else if (count > mCount)
			pvIncCount(count, multiItemCreator);
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
		return Shrink(mCount);
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
			Item* segMemory = pvGetSegMemory(segCount);
			try
			{
				std::forward<ItemCreator>(itemCreator)(segMemory);
			}
			catch (...)
			{
				pvFreeSegMemory(segCount, segMemory);
				throw;
			}
			mSegments.AddBackNogrow(segMemory);
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

	template<typename ArgIterator,
		typename = typename std::iterator_traits<ArgIterator>::iterator_category>
	void Insert(size_t index, ArgIterator begin, ArgIterator end)
	{
		if (internal::IsForwardIterator<ArgIterator>::value)
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
		pvDecCount(mCount - count);
	}

	void Remove(size_t index, size_t count = 1)
	{
		ArrayShifter::Remove(*this, index, count);
	}

	template<typename Predicate>
	internal::EnableIf<internal::IsInvocable<const Predicate&, bool, const Item&>::value, size_t>
	Remove(const Predicate& pred)
	{
		return ArrayShifter::Remove(*this, pred);
	}

	template<typename EqualFunc = std::equal_to<Item>>
	bool Contains(const Item& item, const EqualFunc& equalFunc = EqualFunc()) const
	{
		ConstIterator end = GetEnd();
		return std::find_if(GetBegin(), end,
			[&item, &equalFunc] (const Item& thisItem) { return equalFunc(item, thisItem); }) != end;
	}

	template<typename EqualFunc = std::equal_to<Item>>
	bool IsEqual(const SegmentedArray& array, const EqualFunc& equalFunc = EqualFunc()) const
	{
		return GetCount() == array.GetCount() &&
			std::equal(GetBegin(), GetEnd(), array.GetBegin(), equalFunc);
	}

private:
	Item* pvGetSegMemory(size_t segIndex)
	{
		size_t itemCount = Settings::GetItemCount(segIndex);
		if (itemCount > internal::UIntConst::maxSize / sizeof(Item))
			throw std::length_error("Invalid item count");
		MOMO_STATIC_ASSERT(internal::ObjectAlignmenter<Item>::Check(ItemTraits::alignment));
		return MemManagerProxy::template Allocate<Item>(GetMemManager(),
			itemCount * sizeof(Item));
	}

	void pvFreeSegMemory(size_t segIndex, Item* segMemory) noexcept
	{
		size_t itemCount = Settings::GetItemCount(segIndex);
		MemManagerProxy::Deallocate(GetMemManager(), segMemory, itemCount * sizeof(Item));
	}

	Item& pvGetItem(size_t index) const
	{
		MOMO_CHECK(index < mCount);
		size_t segIndex, itemIndex;
		Settings::GetSegItemIndexes(index, segIndex, itemIndex);
		return mSegments[segIndex][itemIndex];
	}

	template<typename MultiItemCreator>
	void pvIncCount(size_t count, const MultiItemCreator& multiItemCreator)
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
					multiItemCreator(segment + itemIndex);
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
				Item* segMemory = pvGetSegMemory(segCount);
				mSegments.AddBackNogrow(segMemory);
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
			pvFreeSegMemory(i, mSegments[i]);
		mSegments.RemoveBack(segCount - segIndex);
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

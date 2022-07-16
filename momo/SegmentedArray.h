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

template<conceptObject TItem, conceptMemManager TMemManager>
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
	static void GetSegmentItemIndexes(size_t index, size_t& segIndex, size_t& segItemIndex) noexcept
	{
		size_t index1 = (index >> logInitialItemCount) + 1;
		size_t index2 = index & ((size_t{1} << logInitialItemCount) - 1);
		size_t logSegItemCount = pvIndexToLogSegmentItemCount(index1);
		size_t segItemIndex1 = index1 & ((size_t{1} << logSegItemCount) - 1);
		size_t segItemIndex2 = index2;
		segIndex = (index1 >> logSegItemCount) + (size_t{1} << logSegItemCount) - 2;
		segItemIndex = (segItemIndex1 << logInitialItemCount) + segItemIndex2;
	}

	static size_t GetIndex(size_t segIndex, size_t segItemIndex) noexcept
	{
		size_t segItemIndex1 = segItemIndex >> logInitialItemCount;
		size_t segItemIndex2 = segItemIndex & ((size_t{1} << logInitialItemCount) - 1);
		size_t logSegItemCount = pvSegmentIndexToLogSegmentItemCount(segIndex);
		size_t index1 = ((segIndex + 2 - (size_t{1} << logSegItemCount)) << logSegItemCount)
			+ segItemIndex1;
		size_t index2 = segItemIndex2;
		size_t index = ((index1 - 1) << logInitialItemCount) + index2;
		return index;
	}

	static size_t GetSegmentItemCount(size_t segIndex) noexcept
	{
		size_t logSegItemCount = pvSegmentIndexToLogSegmentItemCount(segIndex);
		return size_t{1} << (logSegItemCount + logInitialItemCount);
	}

private:
	static size_t pvIndexToLogSegmentItemCount(size_t index1) noexcept
	{
		return std::bit_width(index1) / 2;
	}

	static size_t pvSegmentIndexToLogSegmentItemCount(size_t segIndex) noexcept
	{
		return std::bit_width((segIndex * 2 + 4) / 3) - 1;
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
	static void GetSegmentItemIndexes(size_t index, size_t& segIndex, size_t& segItemIndex) noexcept
	{
		segIndex = index >> logInitialItemCount;
		segItemIndex = index & ((size_t{1} << logInitialItemCount) - 1);
	}

	static size_t GetIndex(size_t segIndex, size_t segItemIndex) noexcept
	{
		return (segIndex << logInitialItemCount) + segItemIndex;
	}

	static size_t GetSegmentItemCount(size_t /*segIndex*/) noexcept
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

template<conceptObject TItem,
	conceptMemManager TMemManager = MemManagerDefault,
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

	template<internal::conceptInputIterator ArgIterator>
	explicit SegmentedArray(ArgIterator begin, ArgIterator end, MemManager memManager = MemManager())
		: SegmentedArray(std::move(memManager))
	{
		try
		{
			typedef typename ItemTraits::template Creator<
				std::iter_reference_t<ArgIterator>> IterCreator;
			for (ArgIterator iter = begin; iter != end; ++iter)
				pvAddBack(IterCreator(GetMemManager(), *iter));
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
		mCount(std::exchange(array.mCount, 0))
	{
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
	requires std::regular_invocable<const MultiItemCreator&, Item*>
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
	MOMO_FRIENDS_SIZE_BEGIN_END(SegmentedArray)

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

	const Item& GetBackItem(size_t revIndex = 0) const
	{
		return pvGetItem(mCount - 1 - revIndex);
	}

	Item& GetBackItem(size_t revIndex = 0)
	{
		return pvGetItem(mCount - 1 - revIndex);
	}

	template<std::invocable<Item*> ItemCreator>
	void AddBackNogrowCrt(ItemCreator&& itemCreator)
	{
		pvAddBackNogrow(std::forward<ItemCreator>(itemCreator));
	}

	template<typename... ItemArgs>
	//requires requires { typename ItemTraits::template Creator<ItemArgs...>; }
	void AddBackNogrowVar(ItemArgs&&... itemArgs)
	{
		pvAddBackNogrow(typename ItemTraits::template Creator<ItemArgs...>(GetMemManager(),
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

	template<std::invocable<Item*> ItemCreator>
	void AddBackCrt(ItemCreator&& itemCreator)
	{
		pvAddBack(std::forward<ItemCreator>(itemCreator));
	}

	template<typename... ItemArgs>
	//requires requires { typename ItemTraits::template Creator<ItemArgs...>; }
	void AddBackVar(ItemArgs&&... itemArgs)
	{
		pvAddBack(typename ItemTraits::template Creator<ItemArgs...>(GetMemManager(),
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

	template<std::invocable<Item*> ItemCreator>
	void InsertCrt(size_t index, ItemCreator&& itemCreator)
	{
		pvInsert(index, std::forward<ItemCreator>(itemCreator));
	}

	template<typename... ItemArgs>
	//requires requires { typename ItemTraits::template Creator<ItemArgs...>; }
	void InsertVar(size_t index, ItemArgs&&... itemArgs)
	{
		pvInsert(index, typename ItemTraits::template Creator<ItemArgs...>(GetMemManager(),
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
		if constexpr (internal::conceptIterator<ArgIterator, std::forward_iterator_tag>)
		{
			size_t count = internal::UIntMath<>::Dist(begin, end);
			Reserve(mCount + count);
			ArrayShifter::Insert(*this, index, begin, count);
		}
		else
		{
			ArrayShifter::Insert(*this, index, begin, end);
		}
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
	requires std::predicate<const Predicate&, const Item&>
	size_t Remove(const Predicate& pred)
	{
		return ArrayShifter::Remove(*this, pred);
	}

	size_t GetSegmentCount() const noexcept
	{
		return mSegments.GetCount();
	}

	const Item* GetSegmentItems(size_t segIndex) const
	{
		return pvGetSegmentItems(segIndex);
	}

	Item* GetSegmentItems(size_t segIndex)
	{
		return pvGetSegmentItems(segIndex);
	}

	template<typename ItemArg,
		internal::conceptEqualFunc<Item, ItemArg> EqualFunc = std::equal_to<>>
	bool Contains(const ItemArg& itemArg, const EqualFunc& equalFunc = EqualFunc()) const
	{
		return std::any_of(GetBegin(), GetEnd(),
			[&itemArg, &equalFunc] (const Item& item) { return equalFunc(item, itemArg); });
	}

	template<internal::conceptEqualFunc<Item> EqualFunc = std::equal_to<Item>>
	bool IsEqual(const SegmentedArray& array, const EqualFunc& equalFunc = EqualFunc()) const
	{
		return GetCount() == array.GetCount() &&
			std::equal(GetBegin(), GetEnd(), array.GetBegin(), equalFunc);
	}

private:
	Item* pvAllocateSegment(size_t segIndex)
	{
		size_t segItemCount = Settings::GetSegmentItemCount(segIndex);
		if (segItemCount > internal::UIntConst::maxSize / sizeof(Item))
			throw std::bad_array_new_length();
		static_assert(internal::ObjectAlignmenter<Item>::Check(ItemTraits::alignment));
		return MemManagerProxy::template Allocate<Item>(GetMemManager(),
			segItemCount * sizeof(Item));
	}

	void pvDeallocateSegment(size_t segIndex, Item* segment) noexcept
	{
		size_t segItemCount = Settings::GetSegmentItemCount(segIndex);
		MemManagerProxy::Deallocate(GetMemManager(), segment, segItemCount * sizeof(Item));
	}

	Item& pvGetItem(size_t index) const
	{
		MOMO_CHECK(index < mCount);
		size_t segIndex, segItemIndex;
		Settings::GetSegmentItemIndexes(index, segIndex, segItemIndex);
		return mSegments[segIndex][segItemIndex];
	}

	template<typename ItemCreator>
	void pvAddBackNogrow(ItemCreator itemCreator)
	{
		size_t segIndex, segItemIndex;
		Settings::GetSegmentItemIndexes(mCount, segIndex, segItemIndex);
		MOMO_CHECK(segIndex < mSegments.GetCount());
		std::move(itemCreator)(mSegments[segIndex] + segItemIndex);
		++mCount;
	}

	template<typename ItemCreator>
	void pvAddBack(ItemCreator itemCreator)
	{
		size_t segIndex, segItemIndex;
		Settings::GetSegmentItemIndexes(mCount, segIndex, segItemIndex);
		size_t segCount = mSegments.GetCount();
		if (segIndex < segCount)
		{
			std::move(itemCreator)(mSegments[segIndex] + segItemIndex);
		}
		else
		{
			MOMO_ASSERT(segItemIndex == 0);
			mSegments.Reserve(segCount + 1);
			Item* segment = pvAllocateSegment(segCount);
			try
			{
				std::move(itemCreator)(segment);
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

	template<typename ItemCreator>
	void pvInsert(size_t index, ItemCreator itemCreator)
	{
		ItemHandler itemHandler(GetMemManager(), std::move(itemCreator));
		Reserve(mCount + 1);
		ArrayShifter::Insert(*this, index, std::make_move_iterator(&itemHandler), 1);
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
			size_t segIndex, segItemIndex;
			Settings::GetSegmentItemIndexes(mCount, segIndex, segItemIndex);
			while (mCount < count)
			{
				Item* segment = mSegments[segIndex];
				size_t segItemCount = Settings::GetSegmentItemCount(segIndex);
				for (; segItemIndex < segItemCount && mCount < count; ++segItemIndex, ++mCount)
					multiItemCreator(segment + segItemIndex);
				if (segItemIndex == segItemCount)
				{
					++segIndex;
					segItemIndex = 0;
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
		size_t segIndex, segItemIndex;
		Settings::GetSegmentItemIndexes(mCount, segIndex, segItemIndex);
		MemManager& memManager = GetMemManager();
		while (mCount > count)
		{
			if (segItemIndex == 0)
			{
				--segIndex;
				segItemIndex = Settings::GetSegmentItemCount(segIndex);
			}
			size_t remCount = std::minmax(segItemIndex, mCount - count).first;
			ItemTraits::Destroy(memManager, mSegments[segIndex] + segItemIndex - remCount, remCount);
			segItemIndex -= remCount;
			mCount -= remCount;
		}
	}

	void pvIncCapacity(size_t initCapacity, size_t capacity)
	{
		MOMO_ASSERT(capacity >= initCapacity);
		size_t segIndex, segItemIndex;
		Settings::GetSegmentItemIndexes(capacity, segIndex, segItemIndex);
		if (segItemIndex > 0)
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
		size_t segIndex, segItemIndex;
		Settings::GetSegmentItemIndexes(capacity, segIndex, segItemIndex);
		if (segItemIndex > 0)
			++segIndex;
		size_t segCount = mSegments.GetCount();
		for (size_t i = segIndex; i < segCount; ++i)
			pvDeallocateSegment(i, mSegments[i]);
		mSegments.RemoveBack(segCount - segIndex);
	}

	Item* pvGetSegmentItems(size_t segIndex) const
	{
		MOMO_CHECK(segIndex < GetSegmentCount());
		return mSegments[segIndex];
	}

private:
	Segments mSegments;
	size_t mCount;
};

template<conceptObject TItem,
	conceptMemManager TMemManager = MemManagerDefault,
	typename TItemTraits = SegmentedArrayItemTraits<TItem, TMemManager>>
using SegmentedArraySqrt = SegmentedArray<TItem, TMemManager, TItemTraits,
	SegmentedArraySettings<SegmentedArrayItemCountFunc::sqrt>>;

} // namespace momo

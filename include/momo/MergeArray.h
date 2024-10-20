/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
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

template<conceptObject TItem, conceptMemManager TMemManager>
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
	template<internal::conceptIncIterator<Item> Iterator>
	static void Destroy(MemManager& memManager, Iterator begin, size_t count) noexcept
	{
		ItemManager::Destroy(memManager, begin, count);
	}

	template<internal::conceptIncIterator<Item> SrcIterator,
		internal::conceptIncIterator<Item> DstIterator>
	static void Relocate(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
		size_t count)
	{
		ItemManager::Relocate(memManager, srcBegin, dstBegin, count);
	}

	template<internal::conceptIncIterator<Item> SrcIterator,
		internal::conceptIncIterator<Item> DstIterator,
		internal::conceptObjectCreator<Item> ItemCreator>
	static void RelocateCreate(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
		size_t count, FastMovableFunctor<ItemCreator> itemCreator, Item* newItem)
	{
		ItemManager::RelocateCreate(memManager, srcBegin, dstBegin, count,
			std::move(itemCreator), newItem);
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

template<conceptObject TItem,
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
		if constexpr (std::forward_iterator<ArgIterator>)
			pvInitCapacity(internal::UIntMath<>::Dist(begin, end));
		try
		{
			for (ArgIterator iter = begin; iter != end; ++iter)
			{
				if constexpr (std::forward_iterator<ArgIterator>)
					pvAddBackNogrow(FastMovableFunctor(IterCreator(thisMemManager, *iter)));
				else
					pvAddBack(FastMovableFunctor(IterCreator(thisMemManager, *iter)));
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
		mCount(std::exchange(array.mCount, 0)),
		mCapacity(std::exchange(array.mCapacity, 0))
	{
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

	template<internal::conceptObjectMultiCreator<Item> MultiItemCreator>
	static MergeArray CreateCrt(size_t count, MultiItemCreator multiItemCreator,
		MemManager memManager = MemManager())
	{
		FastCopyableFunctor<MultiItemCreator> fastMultiItemCreator(multiItemCreator);
		MergeArray array = CreateCap(count, std::move(memManager));
		for (size_t i = 0; i < count; ++i)
			array.pvAddBackNogrow(FastMovableFunctor(FastCopyableFunctor(fastMultiItemCreator)));
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

	template<internal::conceptObjectMultiCreator<Item> MultiItemCreator>
	void SetCountCrt(size_t count, MultiItemCreator multiItemCreator)
	{
		pvSetCount(count, FastCopyableFunctor<MultiItemCreator>(multiItemCreator));
	}

	void SetCount(size_t count)
	{
		typedef typename ItemTraits::template Creator<> ItemCreator;
		MemManager& memManager = GetMemManager();
		auto multiItemCreator = [&memManager] (Item* newItem)
			{ (ItemCreator(memManager))(newItem); };
		pvSetCount(count, FastCopyableFunctor(multiItemCreator));
	}

	void SetCount(size_t count, const Item& item)
	{
		typedef typename ItemTraits::template Creator<const Item&> ItemCreator;
		MemManager& memManager = GetMemManager();
		auto multiItemCreator = [&memManager, &item] (Item* newItem)
			{ ItemCreator(memManager, item)(newItem); };
		pvSetCount(count, FastCopyableFunctor(multiItemCreator));
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
		size_t segIndex = pvGetSegmentIndex(mCapacity - 1, capacity);
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
		size_t segIndex = pvGetSegmentIndex(mCapacity - 1, initCapacity);
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

	const Item& GetBackItem(size_t revIndex = 0) const
	{
		return pvGetItem(mCount - 1 - revIndex);
	}

	Item& GetBackItem(size_t revIndex = 0)
	{
		return pvGetItem(mCount - 1 - revIndex);
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void AddBackNogrowCrt(ItemCreator itemCreator)
	{
		MOMO_CHECK(mCount < mCapacity);
		pvAddBackNogrow(FastMovableFunctor<ItemCreator>(std::forward<ItemCreator>(itemCreator)));
	}

	template<typename... ItemArgs>
	//requires requires { typename ItemTraits::template Creator<ItemArgs...>; }
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

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void AddBackCrt(ItemCreator itemCreator)
	{
		pvAddBack(FastMovableFunctor<ItemCreator>(std::forward<ItemCreator>(itemCreator)));
	}

	template<typename... ItemArgs>
	//requires requires { typename ItemTraits::template Creator<ItemArgs...>; }
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

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void InsertCrt(size_t index, ItemCreator itemCreator)
	{
		pvInsert(index, FastMovableFunctor<ItemCreator>(std::forward<ItemCreator>(itemCreator)));
	}

	template<typename... ItemArgs>
	//requires requires { typename ItemTraits::template Creator<ItemArgs...>; }
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
		typedef typename ItemTraits::template Creator<const Item&> ItemCreator;
		MemManager& memManager = GetMemManager();
		ItemHandler itemHandler(memManager, FastMovableFunctor(ItemCreator(memManager, item)));
		Reserve(mCount + count);
		ArrayShifter::Insert(*this, index, count, *&itemHandler);
	}

	template<internal::conceptInputIterator ArgIterator>
	void Insert(size_t index, ArgIterator begin, ArgIterator end)
	{
		if constexpr (std::forward_iterator<ArgIterator>)
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
		pvRemoveBack(count);
	}

	void Remove(size_t index, size_t count = 1)
	{
		ArrayShifter::Remove(*this, index, count);
	}

	template<internal::conceptObjectPredicate<Item> ItemFilter>
	size_t Remove(ItemFilter itemFilter)
	{
		return ArrayShifter::Remove(*this, FastCopyableFunctor<ItemFilter>(itemFilter));
	}

	size_t GetSegmentCount() const noexcept
	{
		//return mSegments.GetCount();
		return (mCapacity > 0) ? pvGetSegmentIndex(0, mCapacity) + 1 : 0;
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
	bool Contains(const ItemArg& itemArg, EqualFunc equalFunc = EqualFunc()) const
	{
		FastCopyableFunctor<EqualFunc> fastEqualFunc(equalFunc);
		auto itemPred = [&itemArg, fastEqualFunc] (const Item& item)
			{ return fastEqualFunc(item, itemArg); };
		return std::any_of(GetBegin(), GetEnd(), FastCopyableFunctor(itemPred));
	}

	template<internal::conceptEqualFunc<Item> EqualFunc = std::equal_to<Item>>
	bool IsEqual(const MergeArray& array, EqualFunc equalFunc = EqualFunc()) const
	{
		return std::equal(GetBegin(), GetEnd(), array.GetBegin(), array.GetEnd(),
			FastCopyableFunctor<EqualFunc>(equalFunc));
	}

private:
	static size_t pvGetSegmentItemCount(size_t segIndex) noexcept
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

	static size_t pvGetSegmentIndex(size_t index, size_t capacity) noexcept
	{
		MOMO_ASSERT(index < capacity);
		return static_cast<size_t>(std::bit_width((index ^ (capacity - 1)) >> logInitialItemCount));
	}

	static size_t pvCeilCapacity(size_t capacity) noexcept
	{
		return internal::UIntMath<>::Ceil(capacity, initialItemCount);
	}

	Item* pvAllocateSegment(size_t segIndex)
	{
		size_t segItemCount = pvGetSegmentItemCount(segIndex);
		if (segItemCount > internal::UIntConst::maxSize / sizeof(Item))
			throw std::bad_array_new_length();
		static_assert(internal::ObjectAlignmenter<Item>::Check(ItemTraits::alignment));
		return MemManagerProxy::template Allocate<Item>(GetMemManager(),
			segItemCount * sizeof(Item));
	}

	void pvDeallocateSegment(size_t segIndex, Item* segment) noexcept
	{
		size_t segItemCount = pvGetSegmentItemCount(segIndex);
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
		size_t segCount = pvGetSegmentIndex(0, capacity) + 1;
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

	template<internal::conceptObjectMultiCreator<Item> MultiItemCreator>
	void pvSetCount(size_t count, FastCopyableFunctor<MultiItemCreator> multiItemCreator)
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
					pvAddBackNogrow(FastMovableFunctor(FastCopyableFunctor(multiItemCreator)));
			}
			catch (...)
			{
				pvRemoveBack(mCount - initCount);
				throw;
			}
		}
	}

	Item& pvGetItem(size_t index) const
	{
		MOMO_CHECK(index < mCount);
		return *pvGetItemPtr(index);
	}

	Item* pvGetItemPtr(size_t index) const noexcept
	{
		size_t segIndex = pvGetSegmentIndex(index, mCapacity);
		size_t segItemIndex = index & ((initialItemCount << segIndex) - 1);
		return mSegments[segIndex] + segItemIndex;
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void pvAddBackNogrow(FastMovableFunctor<ItemCreator> itemCreator)
	{
		std::move(itemCreator)(pvGetItemPtr(mCount));
		++mCount;
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void pvAddBack(FastMovableFunctor<ItemCreator> itemCreator)
	{
		if (mCount < mCapacity)
		{
			std::move(itemCreator)(pvGetItemPtr(mCount));
		}
		else if ((mCount & initialItemCount) > 0 || mCount == 0)
		{
			if (mSegments.GetCount() <= 1)
				mSegments.SetCount(2, nullptr);
			Item* segment0 = pvAllocateSegment(0);
			try
			{
				std::move(itemCreator)(segment0);
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
				size_t segItemCount = pvGetSegmentItemCount(segIndex);
				ItemTraits::RelocateCreate(GetMemManager(), pvMakeIterator(mCount - segItemCount),
					mSegments[segIndex], segItemCount, std::move(itemCreator), segment0);
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

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void pvInsert(size_t index, FastMovableFunctor<ItemCreator> itemCreator)
	{
		ItemHandler itemHandler(GetMemManager(), std::move(itemCreator));
		Reserve(mCount + 1);
		ArrayShifter::Insert(*this, index, std::make_move_iterator(&itemHandler), 1);
	}

	void pvRemoveBack(size_t count) noexcept
	{
		ItemTraits::Destroy(GetMemManager(), pvMakeIterator(mCount - count), count);
		mCount -= count;
	}

	Item* pvGetSegmentItems(size_t segIndex) const
	{
		MOMO_CHECK(segIndex < GetSegmentCount());
		return mSegments[segIndex];
	}

private:
	Segments mSegments;
	size_t mCount;
	size_t mCapacity;
};

} // namespace momo

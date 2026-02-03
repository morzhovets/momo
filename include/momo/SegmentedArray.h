/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/SegmentedArray.h

  namespace momo:
    class SegmentedArrayItemTraits
    enum class SegmentedArrayItemCountFunc
    class SegmentedArraySettings
    class SegmentedArrayCore
    class SegmentedArray
    class SegmentedArraySqrt

\**********************************************************/

#pragma once

#include "Array.h"

namespace momo
{

namespace internal
{
	class SegmentedArraySettingsBase
	{
	public:
		static const CheckMode checkMode = CheckMode::bydefault;
		static const bool allowExceptionSuppression = true;

		typedef ArraySettings<> SegmentsSettings;
	};
}

template<conceptObject TItem,
	conceptMemManager TMemManager = MemManagerDefault>
class SegmentedArrayItemTraits
{
public:
	typedef TItem Item;
	typedef TMemManager MemManager;

	//template<typename... ItemArgs>
	//using Creator = typename ItemManager::template Creator<ItemArgs...>;
	template<typename... ItemArgs>
	using Creator = internal::ObjectCreator<Item, MemManager, std::index_sequence_for<ItemArgs...>, ItemArgs...>;

private:
	typedef internal::ObjectManager<Item, MemManager> ItemManager;

public:
	static consteval size_t GetAlignment() noexcept
	{
		return ItemManager::alignment;
	}

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
	: public internal::SegmentedArraySettingsBase
{
public:
	static const SegmentedArrayItemCountFunc itemCountFunc = SegmentedArrayItemCountFunc::sqrt;
	static const size_t logInitialItemCount = tLogInitialItemCount;

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
		return static_cast<size_t>(std::bit_width(index1) / 2);
	}

	static size_t pvSegmentIndexToLogSegmentItemCount(size_t segIndex) noexcept
	{
		return static_cast<size_t>(std::bit_width((segIndex * 2 + 4) / 3) - 1);
	}
};

template<size_t tLogInitialItemCount>
class SegmentedArraySettings<SegmentedArrayItemCountFunc::cnst, tLogInitialItemCount>
	: public internal::SegmentedArraySettingsBase
{
public:
	static const SegmentedArrayItemCountFunc itemCountFunc = SegmentedArrayItemCountFunc::cnst;
	static const size_t logInitialItemCount = tLogInitialItemCount;

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
	All `SegmentedArrayCore` functions and constructors have strong exception safety,
	but not the following cases:
	 - Functions `Insert`, `InsertVar`, `InsertCrt`, `Remove` have
	basic exception safety.

	Swap and move operations invalidate all container iterators.
*/

template<typename TItemTraits,
	typename TSettings = SegmentedArraySettings<>>
class MOMO_EMPTY_BASES SegmentedArrayCore
	: public internal::ArrayBase,
	public internal::Swappable<SegmentedArrayCore>
{
public:
	typedef TItemTraits ItemTraits;
	typedef TSettings Settings;
	typedef typename ItemTraits::Item Item;
	typedef typename ItemTraits::MemManager MemManager;

	typedef internal::ArrayIndexIterator<SegmentedArrayCore, Item> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

private:
	typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

	typedef internal::NestedArraySettings<typename Settings::SegmentsSettings,
		Settings::allowExceptionSuppression> SegmentsSettings;

	typedef ArrayCore<ArrayItemTraits<Item*, MemManager>, SegmentsSettings> Segments;

public:
	SegmentedArrayCore() noexcept(noexcept(MemManager()))
		: SegmentedArrayCore(MemManager())
	{
	}

	explicit SegmentedArrayCore(MemManager memManager) noexcept
		: mSegments(std::move(memManager)),
		mCount(0)
	{
	}

	explicit SegmentedArrayCore(size_t count, MemManager memManager = MemManager())
		: SegmentedArrayCore(std::move(memManager))
	{
		this->SetCount(count);
	}

	explicit SegmentedArrayCore(size_t count, const Item& item, MemManager memManager = MemManager())
		: SegmentedArrayCore(std::move(memManager))
	{
		this->SetCount(count, item);
	}

	template<std::input_iterator ArgIterator, internal::conceptSentinel<ArgIterator> ArgSentinel>
	explicit SegmentedArrayCore(ArgIterator begin, ArgSentinel end, MemManager memManager = MemManager())
		: SegmentedArrayCore(std::move(memManager))
	{
		typedef typename ItemTraits::template Creator<std::iter_reference_t<ArgIterator>> IterCreator;
		MemManager& thisMemManager = GetMemManager();
		for (ArgIterator iter = std::move(begin); iter != end; ++iter)
			pvAddBack(FastMovableFunctor(IterCreator(thisMemManager, *iter)));
	}

	SegmentedArrayCore(std::initializer_list<Item> items)
		: SegmentedArrayCore(items, MemManager())
	{
	}

	explicit SegmentedArrayCore(std::initializer_list<Item> items, MemManager memManager)
		: SegmentedArrayCore(items.begin(), items.end(), std::move(memManager))
	{
	}

	SegmentedArrayCore(SegmentedArrayCore&& array) noexcept
		: mSegments(std::move(array.mSegments)),
		mCount(std::exchange(array.mCount, 0))
	{
	}

	SegmentedArrayCore(const SegmentedArrayCore& array)
		: SegmentedArrayCore(array, true)
	{
	}

	explicit SegmentedArrayCore(const SegmentedArrayCore& array, bool shrink)
		: SegmentedArrayCore(MemManager(array.GetMemManager()))
	{
		pvIncCapacity(0, shrink ? array.GetCount() : array.GetCapacity());
		for (const Item& item : array)
			this->AddBackNogrow(item);
	}

	explicit SegmentedArrayCore(const SegmentedArrayCore& array, MemManager memManager)
		: SegmentedArrayCore(array.GetBegin(), array.GetEnd(), std::move(memManager))
	{
	}

	static SegmentedArrayCore CreateCap(size_t capacity, MemManager memManager = MemManager())
	{
		SegmentedArrayCore array(std::move(memManager));
		array.pvIncCapacity(0, capacity);
		return array;
	}

	template<internal::conceptObjectMultiCreator<Item> ItemMultiCreator>
	static SegmentedArrayCore CreateCrt(size_t count, ItemMultiCreator itemMultiCreator,
		MemManager memManager = MemManager())
	{
		SegmentedArrayCore array = CreateCap(count, std::move(memManager));
		array.pvIncCount(count, FastCopyableFunctor(itemMultiCreator));
		return array;
	}

	~SegmentedArrayCore() noexcept
	{
		pvDecCount(0);
		pvDecCapacity(0);
	}

	SegmentedArrayCore& operator=(SegmentedArrayCore&& array) noexcept
	{
		return internal::ContainerAssigner::Move(std::move(array), *this);
	}

	SegmentedArrayCore& operator=(const SegmentedArrayCore& array)
	{
		return internal::ContainerAssigner::Copy(array, *this);
	}

	void Swap(SegmentedArrayCore& array) noexcept
	{
		mSegments.Swap(array.mSegments);
		std::swap(mCount, array.mCount);
	}

	ConstIterator GetBegin() const noexcept
	{
		return internal::ProxyConstructor<ConstIterator>(this, size_t{0});
	}

	Iterator GetBegin() noexcept
	{
		return internal::ProxyConstructor<Iterator>(this, size_t{0});
	}

	ConstIterator GetEnd() const noexcept
	{
		return internal::ProxyConstructor<ConstIterator>(this, mCount);
	}

	Iterator GetEnd() noexcept
	{
		return internal::ProxyConstructor<Iterator>(this, mCount);
	}

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

	template<internal::conceptObjectMultiCreator<Item> ItemMultiCreator>
	void SetCountCrt(size_t count, ItemMultiCreator itemMultiCreator)
	{
		pvSetCount(count, FastCopyableFunctor(itemMultiCreator));
	}

	//void SetCount(size_t count)
	//void SetCount(size_t count, const Item& item)

	//bool IsEmpty() const noexcept

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

	void Shrink(size_t capacity = 0) noexcept
	{
		if (GetCapacity() <= capacity)
			return;
		if (capacity < mCount)
			capacity = mCount;
		pvDecCapacity(capacity);
		mSegments.TryShrink();
	}

	bool TryShrink(size_t capacity = 0) noexcept
	{
		Shrink(capacity);
		return true;
	}

	//const Item& operator[](size_t index) const
	//Item& operator[](size_t index)
	template<typename RArray>
	internal::ConstLike<Item, RArray>& operator[](this RArray&& array, size_t index)
	{
		auto& thisArray = static_cast<internal::ConstLike<SegmentedArrayCore, RArray>&>(array);
		MOMO_CHECK(index < thisArray.GetCount());
		size_t segIndex, segItemIndex;
		Settings::GetSegmentItemIndexes(index, segIndex, segItemIndex);
		return thisArray.mSegments[segIndex][segItemIndex];
	}

	//const Item& GetBackItem(size_t revIndex = 0) const
	//Item& GetBackItem(size_t revIndex = 0)

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void AddBackNogrowCrt(ItemCreator itemCreator)
	{
		pvAddBackNogrow(FastMovableFunctor(std::forward<ItemCreator>(itemCreator)));
	}

	//template<typename... ItemArgs>
	//void AddBackNogrowVar(ItemArgs&&... itemArgs)

	//void AddBackNogrow(Item&& item)
	//void AddBackNogrow(const Item& item)

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void AddBackCrt(ItemCreator itemCreator)
	{
		pvAddBack(FastMovableFunctor(std::forward<ItemCreator>(itemCreator)));
	}

	//template<typename... ItemArgs>
	//void AddBackVar(ItemArgs&&... itemArgs)

	//void AddBack(Item&& item)
	//void AddBack(const Item& item)

	//template<internal::conceptObjectCreator<Item> ItemCreator>
	//void InsertCrt(size_t index, ItemCreator itemCreator)

	//template<typename... ItemArgs>
	//void InsertVar(size_t index, ItemArgs&&... itemArgs)

	//void Insert(size_t index, Item&& item)
	//void Insert(size_t index, const Item& item)

	//void Insert(size_t index, size_t count, const Item& item)

	//template<std::input_iterator ArgIterator, internal::conceptSentinel<ArgIterator> ArgSentinel>
	//void Insert(size_t index, ArgIterator begin, ArgSentinel end)

	//void Insert(size_t index, std::initializer_list<Item> items)

	void RemoveBack(size_t count = 1)
	{
		MOMO_CHECK(count <= mCount);
		pvDecCount(mCount - count);
	}

	//void Remove(size_t index, size_t count = 1)

	//template<internal::conceptObjectPredicate<Item> ItemFilter>
	//size_t Remove(ItemFilter itemFilter)

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

	//template<internal::conceptEqualComparer<Item> ItemEqualComparer = std::equal_to<Item>>
	//bool IsEqual(const SegmentedArrayCore& array, ItemEqualComparer itemEqualComp = ItemEqualComparer()) const

	//template<typename ItemThreeComparer = internal::TieThreeComparer<Item>>
	//bool Compare(const SegmentedArrayCore& array, ItemThreeComparer itemThreeComp = ItemThreeComparer()) const

	//template<typename ItemArg,
	//	internal::conceptEqualComparer<Item, ItemArg> ItemEqualComparer = std::equal_to<>>
	//bool Contains(const ItemArg& itemArg, ItemEqualComparer itemEqualComp = ItemEqualComparer()) const

private:
	Item* pvAllocateSegment(size_t segIndex)
	{
		size_t segItemCount = Settings::GetSegmentItemCount(segIndex);
		if (segItemCount > internal::UIntConst::maxSize / sizeof(Item))
			MOMO_THROW(std::bad_array_new_length());
		static_assert(internal::ObjectAlignmenter<Item>::Check(ItemTraits::GetAlignment()));
		return MemManagerProxy::template Allocate<Item>(GetMemManager(),
			segItemCount * sizeof(Item));
	}

	void pvDeallocateSegment(size_t segIndex, Item* segment) noexcept
	{
		size_t segItemCount = Settings::GetSegmentItemCount(segIndex);
		MemManagerProxy::Deallocate(GetMemManager(), segment, segItemCount * sizeof(Item));
	}

	template<internal::conceptObjectMultiCreator<Item> ItemMultiCreator>
	void pvSetCount(size_t count, FastCopyableFunctor<ItemMultiCreator> itemMultiCreator)
	{
		if (count < mCount)
			pvDecCount(count);
		else if (count > mCount)
			pvIncCount(count, itemMultiCreator);
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void pvAddBackNogrow(FastMovableFunctor<ItemCreator> itemCreator)
	{
		size_t segIndex, segItemIndex;
		Settings::GetSegmentItemIndexes(mCount, segIndex, segItemIndex);
		MOMO_CHECK(segIndex < mSegments.GetCount());
		std::move(itemCreator)(mSegments[segIndex] + segItemIndex);
		++mCount;
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void pvAddBack(FastMovableFunctor<ItemCreator> itemCreator)
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
			for (internal::Finalizer fin(&SegmentedArrayCore::pvDeallocateSegment, *this, segCount, segment);
				fin; fin.Detach())
			{
				std::move(itemCreator)(segment);
			}
			mSegments.AddBackNogrow(segment);
		}
		++mCount;
	}

	template<internal::conceptObjectMultiCreator<Item> ItemMultiCreator>
	void pvIncCount(size_t count, FastCopyableFunctor<ItemMultiCreator> itemMultiCreator)
	{
		MOMO_ASSERT(count >= mCount);
		size_t initCapacity = GetCapacity();
		size_t initCount = mCount;
		if (count > initCapacity)
			pvIncCapacity(initCapacity, count);
		internal::Finalizer capacityFin(&SegmentedArrayCore::pvDecCapacity, *this, initCapacity);
		internal::Finalizer countFin(&SegmentedArrayCore::pvDecCount, *this, initCount);
		size_t segIndex, segItemIndex;
		Settings::GetSegmentItemIndexes(mCount, segIndex, segItemIndex);
		while (mCount < count)
		{
			Item* segment = mSegments[segIndex];
			size_t segItemCount = Settings::GetSegmentItemCount(segIndex);
			for (; segItemIndex < segItemCount && mCount < count; ++segItemIndex, ++mCount)
				itemMultiCreator(segment + segItemIndex);
			if (segItemIndex == segItemCount)
			{
				++segIndex;
				segItemIndex = 0;
			}
		}
		countFin.Detach();
		capacityFin.Detach();
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
			size_t remCount = internal::UIntMath<>::Min(segItemIndex, mCount - count);
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
		for (internal::Finalizer fin(&SegmentedArrayCore::pvDecCapacity, *this, initCapacity); fin; fin.Detach())
		{
			for (size_t segCount = mSegments.GetCount(); segCount < segIndex; ++segCount)
			{
				mSegments.Reserve(segCount + 1);
				Item* segment = pvAllocateSegment(segCount);
				mSegments.AddBackNogrow(segment);
			}
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
	conceptMemManager TMemManager = MemManagerDefault>
using SegmentedArray = SegmentedArrayCore<SegmentedArrayItemTraits<TItem, TMemManager>>;

template<conceptObject TItem,
	conceptMemManager TMemManager = MemManagerDefault>
using SegmentedArraySqrt = SegmentedArrayCore<SegmentedArrayItemTraits<TItem, TMemManager>,
	SegmentedArraySettings<SegmentedArrayItemCountFunc::sqrt>>;

} // namespace momo

namespace std
{
	template<typename... Params>
	class back_insert_iterator<momo::SegmentedArrayCore<Params...>>
		: public momo::internal::BackInsertIteratorStdBase<momo::SegmentedArrayCore<Params...>>
	{
	private:
		typedef momo::internal::BackInsertIteratorStdBase<momo::SegmentedArrayCore<Params...>>
			BackInsertIteratorStdBase;

	public:
		using BackInsertIteratorStdBase::BackInsertIteratorStdBase;
		using BackInsertIteratorStdBase::operator=;
	};
} // namespace std

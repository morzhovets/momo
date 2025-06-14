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
	: public internal::ArrayBase<TItem, TMemManager, TItemTraits, TSettings>
{
public:
	typedef TItem Item;
	typedef TMemManager MemManager;
	typedef TItemTraits ItemTraits;
	typedef TSettings Settings;

	typedef internal::ArrayIndexIterator<MergeArray, Item> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

private:
	typedef internal::ArrayBase<Item, MemManager, ItemTraits, Settings> ArrayBase;

	typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

	typedef internal::NestedArraySettings<typename Settings::SegmentsSettings> SegmentsSettings;

	typedef Array<Item*, MemManager, ArrayItemTraits<Item*, MemManager>,
		SegmentsSettings> Segments;

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
		for (internal::Finalizer fin = [this] { pvDestroy(); }; fin; fin.Detach())
		{
			for (size_t i = 0; i < count; ++i)
				this->AddBackNogrowVar();
		}
	}

	explicit MergeArray(size_t count, const Item& item, MemManager memManager = MemManager())
		: MergeArray(std::move(memManager))
	{
		pvInitCapacity(count);
		for (internal::Finalizer fin = [this] { pvDestroy(); }; fin; fin.Detach())
		{
			for (size_t i = 0; i < count; ++i)
				this->AddBackNogrow(item);
		}
	}

	template<std::input_iterator ArgIterator, internal::conceptSentinel<ArgIterator> ArgSentinel>
	explicit MergeArray(ArgIterator begin, ArgSentinel end, MemManager memManager = MemManager())
		: MergeArray(std::move(memManager))
	{
		typedef typename ItemTraits::template Creator<std::iter_reference_t<ArgIterator>> IterCreator;
		MemManager& thisMemManager = GetMemManager();
		if constexpr (std::forward_iterator<ArgIterator>)
			pvInitCapacity(internal::UIntMath<>::Dist(begin, end));
		for (internal::Finalizer fin = [this] { pvDestroy(); }; fin; fin.Detach())
		{
			for (ArgIterator iter = std::move(begin); iter != end; ++iter)
			{
				if constexpr (std::forward_iterator<ArgIterator>)
					pvAddBackNogrow(FastMovableFunctor(IterCreator(thisMemManager, *iter)));
				else
					pvAddBack(FastMovableFunctor(IterCreator(thisMemManager, *iter)));
			}
		}
	}

	MergeArray(std::initializer_list<Item> items)
		: MergeArray(items, MemManager())
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
		for (internal::Finalizer fin = [this] { pvDestroy(); }; fin; fin.Detach())
		{
			for (const Item& item : array)
				this->AddBackNogrow(item);
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

	template<internal::conceptObjectMultiCreator<Item> ItemMultiCreator>
	static MergeArray CreateCrt(size_t count, ItemMultiCreator itemMultiCreator,
		MemManager memManager = MemManager())
	{
		FastCopyableFunctor<ItemMultiCreator> fastItemMultiCreator(itemMultiCreator);
		MergeArray array = CreateCap(count, std::move(memManager));
		for (size_t i = 0; i < count; ++i)
			array.pvAddBackNogrow(FastMovableFunctor(FastCopyableFunctor(fastItemMultiCreator)));
		return array;
	}

	~MergeArray() noexcept
	{
		pvDestroy();
	}

	MergeArray& operator=(MergeArray&& array) noexcept
	{
		return internal::ContainerAssigner::Move(std::move(array), *this);
	}

	MergeArray& operator=(const MergeArray& array)
	{
		return internal::ContainerAssigner::Copy(array, *this);
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
		pvSetCount(count, FastCopyableFunctor<ItemMultiCreator>(itemMultiCreator));
	}

	//void SetCount(size_t count)
	//void SetCount(size_t count, const Item& item)

	//bool IsEmpty() const noexcept

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
		if (internal::Finalizer fin = [this] { pvDeallocateSegments(); })
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
			mCapacity = capacity;
		}
	}

	//void Shrink()
	using ArrayBase::Shrink;

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
		if (internal::Finalizer allocReverterFin = [this] { pvDeallocateSegments(); })
		{
			for (internal::Finalizer capacityReverterFin = [this, initCapacity] () noexcept
					{ mCapacity = initCapacity; };
				capacityReverterFin; capacityReverterFin.Detach())
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
		}
	}

	//const Item& operator[](size_t index) const
	//Item& operator[](size_t index)
	template<typename RArray>
	internal::ConstLike<Item, RArray>& operator[](this RArray&& array, size_t index)
	{
		auto& thisArray = static_cast<internal::ConstLike<MergeArray, RArray>&>(array);
		MOMO_CHECK(index < thisArray.GetCount());
		return *thisArray.pvGetItemPtr(index);
	}

	//const Item& GetBackItem(size_t revIndex = 0) const
	//Item& GetBackItem(size_t revIndex = 0)

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void AddBackNogrowCrt(ItemCreator itemCreator)
	{
		MOMO_CHECK(mCount < mCapacity);
		pvAddBackNogrow(FastMovableFunctor<ItemCreator>(std::forward<ItemCreator>(itemCreator)));
	}

	//template<typename... ItemArgs>
	//void AddBackNogrowVar(ItemArgs&&... itemArgs)

	//void AddBackNogrow(Item&& item)
	//void AddBackNogrow(const Item& item)

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void AddBackCrt(ItemCreator itemCreator)
	{
		pvAddBack(FastMovableFunctor<ItemCreator>(std::forward<ItemCreator>(itemCreator)));
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
		pvRemoveBack(count);
	}

	//void Remove(size_t index, size_t count = 1)

	//template<internal::conceptObjectPredicate<Item> ItemFilter>
	//size_t Remove(ItemFilter itemFilter)

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

	//template<typename ItemArg,
	//	internal::conceptEqualComparer<Item, ItemArg> ItemEqualComparer = std::equal_to<>>
	//bool Contains(const ItemArg& itemArg, ItemEqualComparer itemEqualComp = ItemEqualComparer()) const

	//template<internal::conceptEqualComparer<Item> ItemEqualComparer = std::equal_to<Item>>
	//bool IsEqual(const MergeArray& array, ItemEqualComparer itemEqualComp = ItemEqualComparer()) const

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
			MOMO_THROW(std::bad_array_new_length());
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
		for (internal::Finalizer fin = [this] { pvDeallocateSegments(); }; fin; fin.Detach())
		{
			for (size_t i = 0; i < segCount; ++i)
			{
				if (pvHasSegment(i, capacity))
					mSegments[i] = pvAllocateSegment(i);
			}
		}
		mCapacity = capacity;
	}

	Iterator pvMakeIterator(size_t index) noexcept
	{
		return IteratorProxy(this, index);
	}

	template<internal::conceptObjectMultiCreator<Item> ItemMultiCreator>
	void pvSetCount(size_t count, FastCopyableFunctor<ItemMultiCreator> itemMultiCreator)
	{
		if (count <= mCount)
		{
			pvRemoveBack(mCount - count);
		}
		else //if (count <= mCapacity)
		{
			Reserve(count);	//?
			size_t initCount = mCount;
			for (internal::Finalizer fin = [this, initCount] { pvRemoveBack(mCount - initCount); };
				fin; fin.Detach())
			{
				for (size_t i = initCount; i < count; ++i)
					pvAddBackNogrow(FastMovableFunctor(FastCopyableFunctor(itemMultiCreator)));
			}
		}
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
			for (internal::Finalizer fin = [this, segment0] { pvDeallocateSegment(0, segment0); };
				fin; fin.Detach())
			{
				std::move(itemCreator)(segment0);
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
			auto allocReverter = [this, &segment0] () noexcept
			{
				pvDeallocateSegments();
				pvDeallocateSegment(0, segment0);
			};
			if (internal::Finalizer fin = allocReverter)
			{
				mSegments[segIndex] = pvAllocateSegment(segIndex);
				size_t segItemCount = pvGetSegmentItemCount(segIndex);
				ItemTraits::RelocateCreate(GetMemManager(), pvMakeIterator(mCount - segItemCount),
					mSegments[segIndex], segItemCount, std::move(itemCreator), segment0);
				mCapacity += initialItemCount;
				std::swap(segment0, mSegments[0]);
			}
		}
		++mCount;
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

namespace std
{
	template<typename... Params>
	class back_insert_iterator<momo::MergeArray<Params...>>
		: public momo::internal::BackInsertIteratorStdBase<momo::MergeArray<Params...>>
	{
	private:
		typedef momo::internal::BackInsertIteratorStdBase<momo::MergeArray<Params...>>
			BackInsertIteratorStdBase;

	public:
		using BackInsertIteratorStdBase::BackInsertIteratorStdBase;
		using BackInsertIteratorStdBase::operator=;
	};
} // namespace std

/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/MergeSet.h

  namespace momo:
    class MergeSetItemTraits
    class MergeSetSettings
    class MergeSetCore
    class MergeSet
    class MergeSetHash

\**********************************************************/

#pragma once

#include "MergeTraits.h"
#include "SetUtility.h"
#include "IteratorUtility.h"
#include "Array.h"
#include "HashSorter.h"

namespace momo
{

namespace internal
{
	template<typename Item>
	struct MergeSetSegment
	{
		Item* items;
		size_t itemCount;
		bool isLast;
	};

	template<typename Item>
	struct MergeSetItemPtrCode
	{
		Item* itemPtr;
		size_t hashCode;
	};

	template<typename TItem, typename TSettings>
	class MOMO_EMPTY_BASES MergeSetIterator;

	template<typename TItem, typename TSettings>
	class MOMO_EMPTY_BASES MergeSetPosition
		: private VersionKeeper<TSettings>,
		public ForwardIteratorBase
	{
	protected:
		typedef TItem Item;
		typedef TSettings Settings;

		typedef internal::VersionKeeper<Settings> VersionKeeper;

	public:
		typedef const Item& Reference;
		typedef const Item* Pointer;

		typedef MergeSetPosition ConstPosition;

		typedef MergeSetIterator<Item, Settings> Iterator;

	public:
		explicit MergeSetPosition() noexcept
			: mItemPtr(nullptr)
		{
		}

		Pointer operator->() const
		{
			VersionKeeper::Check();
			MOMO_CHECK(mItemPtr != nullptr);
			return mItemPtr;
		}

		friend bool operator==(MergeSetPosition pos1, MergeSetPosition pos2) noexcept
		{
			return pos1.mItemPtr == pos2.mItemPtr;
		}

	protected:
		explicit MergeSetPosition(Item* itemPtr, const size_t* version) noexcept
			: VersionKeeper(version),
			mItemPtr(itemPtr)
		{
		}

		Item* ptGetItemPtr() const noexcept
		{
			return mItemPtr;
		}

		void ptReset(Item* itemPtr) noexcept
		{
			mItemPtr = itemPtr;
		}

	private:
		Item* mItemPtr;
	};

	template<typename TItem, typename TSettings>
	class MergeSetIterator : public MergeSetPosition<TItem, TSettings>
	{
	public:
		typedef MergeSetPosition<TItem, TSettings> Position;

		typedef MergeSetIterator ConstIterator;

	protected:
		using typename Position::Item;
		//using typename Position::Settings;

		typedef MergeSetSegment<Item> Segment;

	public:
		explicit MergeSetIterator() noexcept
			: mSegment(nullptr)
		{
		}

		MergeSetIterator(Position pos) noexcept
			: Position(pos),
			mSegment(nullptr)
		{
		}

		//operator ConstIterator() const noexcept

		MergeSetIterator& operator++()
		{
			Position::operator->();	// check
			if (Item* itemPtr = pvInc(); itemPtr != nullptr)
				Position::ptReset(itemPtr);
			else
				*this = MergeSetIterator();
			return *this;
		}

		using ForwardIteratorBase::operator++;

	protected:
		explicit MergeSetIterator(const Segment* segment, const size_t* version) noexcept
			: Position(segment->items, version),
			mSegment(segment)
		{
			MOMO_ASSERT(segment->itemCount > 0);
		}

	private:
		Item* pvInc() noexcept
		{
			if (mSegment == nullptr)
				return nullptr;
			Item* itemPtr = std::next(Position::ptGetItemPtr());
			if (itemPtr != mSegment->items + mSegment->itemCount)
				return itemPtr;
			while (true)
			{
				if (mSegment->isLast)
					return nullptr;
				--mSegment;
				if (mSegment->itemCount > 0)
					break;
			}
			return mSegment->items;
		}

	private:
		const Segment* mSegment;
	};
}

template<conceptObject TKey,
	conceptMemManager TMemManager = MemManagerDefault>
class MergeSetItemTraits : public internal::SetItemTraits<TKey, TMemManager>
{
private:
	typedef internal::SetItemTraits<TKey, TMemManager> SetItemTraits;

public:
	using typename SetItemTraits::Item;
	using typename SetItemTraits::MemManager;

private:
	typedef internal::ObjectManager<Item, MemManager> ItemManager;

public:
	template<internal::conceptIncIterator<Item> SrcIterator,
		internal::conceptIncIterator<Item> DstIterator,
		internal::conceptObjectCreator<Item> ItemCreator>
	static void RelocateCreate(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
		size_t count, FastMovableFunctor<ItemCreator> itemCreator, Item* newItem)
	{
		ItemManager::RelocateCreate(memManager, srcBegin, dstBegin, count,
			std::move(itemCreator), newItem);
	}
};

class MergeSetSettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION;
	static const bool allowExceptionSuppression = true;
};

/*!
	All `MergeSetCore` functions and constructors have strong exception safety,
	but not the following cases:
	 - Functions `Insert` receiving many items have basic exception safety.
*/

template<typename TItemTraits,
	conceptMergeTraits<typename TItemTraits::Key> TMergeTraits = MergeTraits<typename TItemTraits::Key>,
	typename TSettings = MergeSetSettings>
requires conceptSetItemTraits<TItemTraits, typename TItemTraits::Key, typename TItemTraits::MemManager>
class MOMO_EMPTY_BASES MergeSetCore
	: public internal::Rangeable,
	public internal::Swappable<MergeSetCore>
{
public:
	typedef TItemTraits ItemTraits;
	typedef TMergeTraits MergeTraits;
	typedef TSettings Settings;
	typedef typename ItemTraits::Key Key;
	typedef typename ItemTraits::Item Item;
	typedef typename ItemTraits::MemManager MemManager;

	typedef internal::MergeSetIterator<Item, Settings> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::MergeSetPosition<Item, Settings> Position;
	typedef typename Position::ConstPosition ConstPosition;

	typedef internal::InsertResult<Position> InsertResult;

	typedef internal::SetExtractedItem<ItemTraits, Settings> ExtractedItem;

private:
	typedef typename MergeTraits::BloomFilter BloomFilter;

	static const size_t initialItemCount = size_t{1} << MergeTraits::logInitialItemCount;

	typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

	template<typename... ItemArgs>
	using Creator = typename ItemTraits::template Creator<ItemArgs...>;

	static const bool allowExceptionSuppression = internal::Catcher::allowExceptionSuppression<Settings>;

	typedef internal::SetCrew<MergeTraits, MemManager, Settings::checkVersion> Crew;

	typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

	typedef internal::MergeSetSegment<Item> Segment;
	typedef ArrayCore<ArrayItemTraits<Segment, MemManagerPtr>,
		internal::NestedArraySettings<ArraySettings<0, false>>> Segments;

	typedef internal::MergeSetItemPtrCode<Item> ItemPtrCode;
	typedef internal::NestedArrayIntCap<initialItemCount <= 32 ? 32 : 0,
		Item*, MemManagerPtr> ItemPtrs;
	typedef internal::NestedArrayIntCap<initialItemCount <= 32 ? 32 : 0,	//?
		ItemPtrCode, MemManagerPtr> ItemPtrCodes;

	struct PositionProxy : public Position
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Position)
	};

	struct IteratorProxy : public Iterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
	};

public:
	MergeSetCore()
		: MergeSetCore(MergeTraits())
	{
	}

	explicit MergeSetCore(const MergeTraits& mergeTraits, MemManager memManager = MemManager())
		: mCrew(mergeTraits, std::move(memManager)),
		mCount(0),
		mSegments(MemManagerPtr(GetMemManager()))
	{
	}

	template<internal::conceptSetArgIterator<Item> ArgIterator,
		internal::conceptSentinel<ArgIterator> ArgSentinel>
	explicit MergeSetCore(ArgIterator begin, ArgSentinel end,
		const MergeTraits& mergeTraits = MergeTraits(), MemManager memManager = MemManager())
		: MergeSetCore(mergeTraits, std::move(memManager))
	{
		for (internal::Finalizer fin(&MergeSetCore::pvDestroy, *this); fin; fin.Detach())
			Insert(std::move(begin), std::move(end));
	}

	MergeSetCore(std::initializer_list<Item> items)
		: MergeSetCore(items, MergeTraits())
	{
	}

	explicit MergeSetCore(std::initializer_list<Item> items, const MergeTraits& mergeTraits,
		MemManager memManager = MemManager())
		: MergeSetCore(items.begin(), items.end(), mergeTraits, std::move(memManager))
	{
	}

	MergeSetCore(MergeSetCore&& mergeSet) noexcept
		: mCrew(std::move(mergeSet.mCrew)),
		mCount(std::exchange(mergeSet.mCount, 0)),
		mSegments(std::move(mergeSet.mSegments))
	{
	}

	MergeSetCore(const MergeSetCore& mergeSet)
		: MergeSetCore(mergeSet, MemManager(mergeSet.GetMemManager()))
	{
	}

	explicit MergeSetCore(const MergeSetCore& mergeSet, MemManager memManager)
		: MergeSetCore(mergeSet.GetBegin(), mergeSet.GetEnd(),
			mergeSet.GetMergeTraits(), std::move(memManager))
	{
	}

	~MergeSetCore() noexcept
	{
		pvDestroy();
	}

	MergeSetCore& operator=(MergeSetCore&& mergeSet) noexcept
	{
		return internal::ContainerAssigner::Move(std::move(mergeSet), *this);
	}

	MergeSetCore& operator=(const MergeSetCore& mergeSet)
	{
		return internal::ContainerAssigner::Copy(mergeSet, *this);
	}

	void Swap(MergeSetCore& mergeSet) noexcept
	{
		mCrew.Swap(mergeSet.mCrew);
		std::swap(mCount, mergeSet.mCount);
		mSegments.Swap(mergeSet.mSegments);
	}

	Iterator GetBegin() const noexcept
	{
		if (IsEmpty())
			return Iterator();
		size_t segIndex = mSegments.GetCount() - 1;
		while (mSegments[segIndex].itemCount == 0)
			--segIndex;
		return IteratorProxy(&mSegments[segIndex], mCrew.GetVersion());
	}

	Iterator GetEnd() const noexcept
	{
		return Iterator();
	}

	const MergeTraits& GetMergeTraits() const noexcept
	{
		return mCrew.GetContainerTraits();
	}

	const MemManager& GetMemManager() const noexcept
	{
		return mCrew.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mCrew.GetMemManager();
	}

	size_t GetCount() const noexcept
	{
		return mCount;
	}

	bool IsEmpty() const noexcept
	{
		return mCount == 0;
	}

	void Clear() noexcept
	{
		pvDestroy();
		mCount = 0;
		mCrew.IncVersion();
	}

	Position Find(const Key& key) const
	{
		return pvFind(key);
	}

	bool ContainsKey(const Key& key) const
	{
		return pvFind(key) != Position();
	}

	template<internal::conceptObjectCreator<Item> ItemCreator,
		bool extraCheck = true>
	InsertResult InsertCrt(const Key& key, ItemCreator itemCreator)
	{
		return pvInsert<extraCheck>(key, FastMovableFunctor(std::forward<ItemCreator>(itemCreator)));
	}

	template<typename... ItemArgs>
	requires requires { typename Creator<ItemArgs...>; }
	InsertResult InsertVar(const Key& key, ItemArgs&&... itemArgs)
	{
		return InsertCrt(key,
			Creator<ItemArgs...>(GetMemManager(), std::forward<ItemArgs>(itemArgs)...));
	}

	InsertResult Insert(Item&& item)
	{
		const Key& key = ItemTraits::GetKey(std::as_const(item));
		return InsertCrt<Creator<Item&&>, false>(key,
			Creator<Item&&>(GetMemManager(), std::move(item)));
	}

	InsertResult Insert(const Item& item)
	{
		return InsertCrt<Creator<const Item&>, false>(ItemTraits::GetKey(item),
			Creator<const Item&>(GetMemManager(), item));
	}

	InsertResult Insert(ExtractedItem&& extItem)
	{
		auto itemCreator = [this, &extItem] (Item* newItem)
		{
			auto itemRemover = [this, newItem] (Item& item)
				{ ItemTraits::Relocate(nullptr, &GetMemManager(), item, newItem); };
			extItem.Remove(itemRemover);
		};
		return pvInsert<false>(ItemTraits::GetKey(extItem.GetItem()),
			FastMovableFunctor(std::move(itemCreator)));
	}

	template<internal::conceptSetArgIterator<Item> ArgIterator,
		internal::conceptSentinel<ArgIterator> ArgSentinel>
	size_t Insert(ArgIterator begin, ArgSentinel end)
	{
		size_t initCount = GetCount();
		for (ArgIterator iter = std::move(begin); iter != end; ++iter)
		{
			auto&& ref = *iter;
			InsertVar(ItemTraits::GetKey(std::as_const(ref)), std::forward<decltype(ref)>(ref));
		}
		return GetCount() - initCount;
	}

	size_t Insert(std::initializer_list<Item> items)
	{
		return Insert(items.begin(), items.end());
	}

	template<internal::conceptObjectCreator<Item> ItemCreator,
		bool extraCheck = true>
	Position AddCrt(ConstPosition pos, ItemCreator itemCreator)
	{
		return pvAdd<extraCheck>(pos, FastMovableFunctor(std::forward<ItemCreator>(itemCreator)));
	}

	template<typename... ItemArgs>
	requires requires { typename Creator<ItemArgs...>; }
	Position AddVar(ConstPosition pos, ItemArgs&&... itemArgs)
	{
		return AddCrt(pos,
			Creator<ItemArgs...>(GetMemManager(), std::forward<ItemArgs>(itemArgs)...));
	}

	Position Add(ConstPosition pos, Item&& item)
	{
		return AddVar(pos, std::move(item));
	}

	Position Add(ConstPosition pos, const Item& item)
	{
		return AddVar(pos, item);
	}

private:
	static size_t pvGetSegmentItemCount(size_t segIndex) noexcept
	{
		return initialItemCount << ((segIndex > 0) ? segIndex - 1 : 0);
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

	void pvDeallocateSegment(size_t segIndex, Item* segItems) noexcept
	{
		size_t segItemCount = pvGetSegmentItemCount(segIndex);
		MemManagerProxy::Deallocate(GetMemManager(), segItems, segItemCount * sizeof(Item));
	}

	void pvDeallocateSegments(size_t segCount) noexcept
	{
		for (size_t s = 0; s < segCount; ++s)
		{
			Segment& segment = mSegments[s];
			MOMO_ASSERT(segment.itemCount == 0);
			if (segment.items != nullptr)
				pvDeallocateSegment(s, segment.items);
			segment.items = nullptr;
		}
	}

	void pvDestroy() noexcept
	{
		MemManager& memManager = GetMemManager();
		size_t segCount = mSegments.GetCount();
		for (size_t s = 0; s < segCount; ++s)
		{
			Segment& segment = mSegments[s];
			for (size_t i = 0; i < segment.itemCount; ++i)
				ItemTraits::Destroy(&memManager, segment.items[i]);
			segment.itemCount = 0;
		}
		pvDeallocateSegments(segCount);
		mSegments.Clear(true);
	}

	Position pvMakePosition(Item* itemPtr) const noexcept
	{
		return PositionProxy(itemPtr, mCrew.GetVersion());
	}

	bool pvExtraCheck(ConstPosition pos) const noexcept
	{
		bool res = true;
		if constexpr (allowExceptionSuppression)
		{
			res = false;
			internal::Catcher::CatchAll([this, &res, pos] ()
				{ res = (pos == pvFind(ItemTraits::GetKey(*pos))); });
		}
		return res;
	}

	Position pvFind(const Key& key) const
	{
		const MergeTraits& mergeTraits = GetMergeTraits();
		size_t hashCode = 0;
		if constexpr (MergeTraits::func == MergeTraitsFunc::hash)
			hashCode = mergeTraits.GetHashCode(key);
		auto itemPred = [&mergeTraits, &key] (const Item& item)
			{ return mergeTraits.IsEqual(ItemTraits::GetKey(item), key); };
		for (size_t s = mSegments.GetCount(); s > 1; --s)
		{
			size_t segIndex = s - 1;
			size_t segItemCount = mSegments[segIndex].itemCount;
			if (segItemCount == 0)
				continue;
			Item* segItems = mSegments[segIndex].items;
			if constexpr (MergeTraits::func == MergeTraitsFunc::hash)
			{
				auto hasher = [&mergeTraits] (const Item& item)
					{ return mergeTraits.GetHashCode(ItemTraits::GetKey(item)); };
				auto equalComp = [&mergeTraits] (const Item& item1, const auto& item2)
				{
					const Key& key1 = ItemTraits::GetKey(item1);
					if constexpr (std::is_same_v<decltype(item2), const Key&>)
						return mergeTraits.IsEqual(key1, item2);
					else
						return mergeTraits.IsEqual(key1, ItemTraits::GetKey(item2));
				};
				auto findRes = HashSorter::Find(segItems, segItemCount, key, hashCode, hasher, equalComp);
				if (findRes.found)
					return pvMakePosition(findRes.iterator);
			}
			else
			{
				auto lessComp = [&mergeTraits] (const Item& item1, const Key& key2)
					{ return mergeTraits.IsLess(ItemTraits::GetKey(item1), key2); };
				Item* itemPtr = std::lower_bound(segItems,
					segItems + segItemCount - 1, key, FastCopyableFunctor(lessComp));
				if (itemPred(*itemPtr))
					return pvMakePosition(itemPtr);
			}
		}
		if (!mSegments.IsEmpty())
		{
			Item* segItems = mSegments[0].items;
			size_t segItemCount = mSegments[0].itemCount;
			Item* itemPtr = std::find_if(segItems, segItems + segItemCount, FastCopyableFunctor(itemPred));
			if (itemPtr != segItems + segItemCount)
				return pvMakePosition(itemPtr);
		}
		return Position();
	}

	template<bool extraCheck, internal::conceptObjectCreator<Item> ItemCreator>
	InsertResult pvInsert(const Key& key, FastMovableFunctor<ItemCreator> itemCreator)
	{
		Position pos = pvFind(key);
		if (!!pos)
			return { pos, false };
		pos = pvAdd<extraCheck>(pos, std::move(itemCreator));
		return { pos, true };
	}

	template<bool extraCheck, internal::conceptObjectCreator<Item> ItemCreator>
	Position pvAdd([[maybe_unused]] ConstPosition pos, FastMovableFunctor<ItemCreator> itemCreator)
	{
		MOMO_CHECK(!pos);
		Item* itemPtr = pvAdd(std::move(itemCreator));
		++mCount;
		mCrew.IncVersion();
		Position resPos = pvMakePosition(itemPtr);
		MOMO_EXTRA_CHECK(!extraCheck || pvExtraCheck(resPos));
		return resPos;
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	Item* pvAdd(FastMovableFunctor<ItemCreator> itemCreator)
	{
		if (mSegments.IsEmpty())
		{
			mSegments.SetCount(1, Segment());
			mSegments[0].isLast = true;
		}
		Segment& segment0 = mSegments[0];
		if (segment0.itemCount < initialItemCount)
		{
			if (segment0.items == nullptr)
				segment0.items = pvAllocateSegment(0);
			std::move(itemCreator)(segment0.items + segment0.itemCount);
			++segment0.itemCount;
			return segment0.items + segment0.itemCount - 1;
		}
		else
		{
			return pvAddGrow(std::move(itemCreator));
		}
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	Item* pvAddGrow(FastMovableFunctor<ItemCreator> itemCreator)
	{
		const MergeTraits& mergeTraits = GetMergeTraits();
		MemManager& memManager = GetMemManager();
		size_t relItemCount = initialItemCount;
		size_t segIndex = 1;
		while (true)
		{
			if (segIndex < mSegments.GetCount())
				relItemCount += mSegments[segIndex].itemCount;
			if (relItemCount <= pvGetSegmentItemCount(segIndex))
				break;
			++segIndex;
		}
		if (mSegments.GetCount() <= segIndex)
			mSegments.SetCount(segIndex + 1, Segment());
		Item* newItems0 = pvAllocateSegment(0);
		internal::Finalizer fin0(&MergeSetCore::pvDeallocateSegment, *this, 0, newItems0);
		Item* newItems = pvAllocateSegment(segIndex);
		internal::Finalizer fin(&MergeSetCore::pvDeallocateSegment, *this, segIndex, newItems);
		Item* items0 = mSegments[0].items;
		if constexpr (MergeTraits::func == MergeTraitsFunc::hash)
		{
			ItemPtrCodes itemPtrCodes(relItemCount, MemManagerPtr(memManager));
			for (size_t i = 0; i < initialItemCount; ++i)
			{
				Item* itemPtr = items0 + i;
				itemPtrCodes[relItemCount - initialItemCount + i] =
					{ itemPtr, mergeTraits.GetHashCode(ItemTraits::GetKey(*itemPtr)) };
			}
			auto lessComp = [] (ItemPtrCode itemPtrCode1, ItemPtrCode itemPtrCode2) noexcept
				{ return itemPtrCode1.hashCode < itemPtrCode2.hashCode; };
			pvSortPtrs(&itemPtrCodes[relItemCount - initialItemCount], lessComp);
			size_t curItemCount = initialItemCount;
			for (size_t s = 1; s <= segIndex; ++s)
			{
				size_t segItemCount = mSegments[s].itemCount;
				if (segItemCount == 0)
					continue;
				curItemCount += segItemCount;
				pvMergePtrCodes(mSegments[s].items,
					&itemPtrCodes[relItemCount - curItemCount], segItemCount);
			}
			internal::IncIterator srcIter = [iter = itemPtrCodes.GetItems()] () mutable noexcept
				{ return (iter++)->itemPtr; };
			ItemTraits::RelocateCreate(memManager,
				srcIter, newItems, relItemCount, std::move(itemCreator), newItems0);
		}
		else if constexpr (MergeTraits::func == MergeTraitsFunc::lessNothrow
			&& ItemTraits::isNothrowRelocatable)
		{
			std::move(itemCreator)(newItems0);
			for (size_t i = 0; i < initialItemCount; ++i)
				pvRelocate(items0[i], newItems + relItemCount - initialItemCount + i);
			pvSortRelocate(newItems + relItemCount - initialItemCount);
			size_t curItemCount = initialItemCount;
			for (size_t s = 1; s <= segIndex; ++s)
			{
				size_t segItemCount = mSegments[s].itemCount;
				if (segItemCount == 0)
					continue;
				curItemCount += segItemCount;
				pvMergeRelocate(mSegments[s].items,
					newItems + relItemCount - curItemCount, segItemCount);
			}
		}
		else
		{
			ItemPtrs itemPtrs(relItemCount, MemManagerPtr(memManager));
			for (size_t i = 0; i < initialItemCount; ++i)
				itemPtrs[relItemCount - initialItemCount + i] = items0 + i;
			auto lessComp = [&mergeTraits] (Item* itemPtr1, Item* itemPtr2)
			{
				return mergeTraits.IsLess(ItemTraits::GetKey(*itemPtr1), ItemTraits::GetKey(*itemPtr2));
			};
			pvSortPtrs(&itemPtrs[relItemCount - initialItemCount], lessComp);
			size_t curItemCount = initialItemCount;
			for (size_t s = 1; s <= segIndex; ++s)
			{
				size_t segItemCount = mSegments[s].itemCount;
				if (segItemCount == 0)
					continue;
				curItemCount += segItemCount;
				pvMergePtrs(mSegments[s].items, &itemPtrs[relItemCount - curItemCount], segItemCount);
			}
			internal::IncIterator srcIter = [iter = itemPtrs.GetItems()] () mutable noexcept
				{ return *iter++; };
			ItemTraits::RelocateCreate(memManager,
				srcIter, newItems, relItemCount, std::move(itemCreator), newItems0);
		}
		for (size_t s = 0; s <= segIndex; ++s)
			mSegments[s].itemCount = 0;
		pvDeallocateSegments(segIndex + 1);
		mSegments[segIndex].items = newItems;
		mSegments[segIndex].itemCount = relItemCount;
		fin.Detach();
		mSegments[0].items = newItems0;
		mSegments[0].itemCount = 1;
		fin0.Detach();
		return newItems0;
	}

	void pvSortRelocate(Item* items) noexcept
	{
		const MergeTraits& mergeTraits = GetMergeTraits();
		for (size_t i = 1; i < initialItemCount; ++i)
		{
			internal::ObjectBuffer<Item, ItemTraits::alignment> itemBuffer;
			pvRelocate(items[i], itemBuffer.GetPtr());
			const Key& key = ItemTraits::GetKey(itemBuffer.Get());
			size_t j = i;
			for (; j > 0; --j)
			{
				if (!mergeTraits.IsLess(key, ItemTraits::GetKey(items[j - 1])))
					break;
				pvRelocate(items[j - 1], items + j);
			}
			pvRelocate(itemBuffer.Get(), items + j);
		}
	}
		
	template<typename ItemPtr, typename LessComparer>
	void pvSortPtrs(ItemPtr* itemPtrs, const LessComparer& lessComp)
	{
		for (size_t i = 1; i < initialItemCount; ++i)
		{
			ItemPtr itemPtr = itemPtrs[i];
			size_t j = i;
			for (; j > 0; --j)
			{
				if (!lessComp(itemPtr, itemPtrs[j - 1]))
					break;
				itemPtrs[j] = itemPtrs[j - 1];
			}
			itemPtrs[j] = itemPtr;
		}
	}

	void pvMergeRelocate(Item* srcItems1, Item* dstItems, size_t count)
	{
		const MergeTraits& mergeTraits = GetMergeTraits();
		Item* srcItems2 = dstItems + count;
		size_t srcIndex1 = 0;
		size_t srcIndex2 = 0;
		size_t dstIndex = 0;
		while (srcIndex1 < count)
		{
			if (srcIndex2 < count && mergeTraits.IsLess(
				ItemTraits::GetKey(srcItems2[srcIndex2]),
				ItemTraits::GetKey(srcItems1[srcIndex1])))
			{
				pvRelocate(srcItems2[srcIndex2], dstItems + dstIndex);
				++srcIndex2;
			}
			else
			{
				pvRelocate(srcItems1[srcIndex1], dstItems + dstIndex);
				++srcIndex1;
			}
			++dstIndex;
		}
	}

	void pvMergePtrs(Item* srcItems1, Item** dstItemPtrs, size_t count)
	{
		const MergeTraits& mergeTraits = GetMergeTraits();
		Item** srcItemPtrs2 = dstItemPtrs + count;
		size_t srcIndex1 = 0;
		size_t srcIndex2 = 0;
		size_t dstIndex = 0;
		while (srcIndex1 < count)
		{
			if (srcIndex2 < count && mergeTraits.IsLess(
				ItemTraits::GetKey(*srcItemPtrs2[srcIndex2]),
				ItemTraits::GetKey(srcItems1[srcIndex1])))
			{
				dstItemPtrs[dstIndex] = srcItemPtrs2[srcIndex2];
				++srcIndex2;
			}
			else
			{
				dstItemPtrs[dstIndex] = srcItems1 + srcIndex1;
				++srcIndex1;
			}
			++dstIndex;
		}
	}

	void pvMergePtrCodes(Item* srcItems1, ItemPtrCode* dstItemPtrCodes, size_t count)
	{
		const MergeTraits& mergeTraits = GetMergeTraits();
		ItemPtrCode* srcItemPtrCodes2 = dstItemPtrCodes + count;
		size_t srcIndex1 = 0;
		size_t srcIndex2 = 0;
		size_t dstIndex = 0;
		while (srcIndex1 < count)
		{
			size_t srcHashCode1 = mergeTraits.GetHashCode(ItemTraits::GetKey(srcItems1[srcIndex1]));
			while (srcIndex2 < count && srcItemPtrCodes2[srcIndex2].hashCode < srcHashCode1)
			{
				dstItemPtrCodes[dstIndex] = srcItemPtrCodes2[srcIndex2];
				++srcIndex2;
				++dstIndex;
			}
			dstItemPtrCodes[dstIndex] = { srcItems1 + srcIndex1, srcHashCode1 };
			++srcIndex1;
			++dstIndex;
		}
	}

	void pvRelocate(Item& srcItem, Item* dstItem) noexcept(ItemTraits::isNothrowRelocatable)
	{
		MemManager& memManager = GetMemManager();
		ItemTraits::Relocate(&memManager, &memManager, srcItem, dstItem);
	}

private:
	MOMO_NO_UNIQUE_ADDRESS Crew mCrew;
	size_t mCount;
	Segments mSegments;
	//MOMO_NO_UNIQUE_ADDRESS BloomFilter mBloomFilter;
};

template<conceptObject TKey,
	conceptMergeTraits<TKey> TMergeTraits = MergeTraits<TKey>,
	conceptMemManager TMemManager = MemManagerDefault>
using MergeSet = MergeSetCore<MergeSetItemTraits<TKey, TMemManager>, TMergeTraits>;

template<conceptObject TKey>
using MergeSetHash = MergeSet<TKey, MergeTraitsHash<TKey>>;

} // namespace momo

namespace std
{
	template<typename AI, typename S>
	struct iterator_traits<momo::internal::MergeSetIterator<AI, S>>
		: public momo::internal::IteratorTraitsStd<momo::internal::MergeSetIterator<AI, S>,
			forward_iterator_tag>
	{
	};
} // namespace std

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
		uint8_t* itemFlags;
		size_t capacity;
		size_t count;
		bool isLast;
	};

	template<typename Item>
	struct MergeSetItemPtrCode
	{
		Item* itemPtr;
		size_t hashCode;
	};

	template<typename TItem>
	class MergeSetSegmentByteIterator : public ArrayIteratorBase
	{
	public:
		typedef TItem Item;

		typedef std::byte& Reference;
		typedef std::byte* Pointer;

	public:
		explicit MergeSetSegmentByteIterator(Item* itemPtr = nullptr) noexcept
			: mItemPtr(itemPtr)
		{
		}

		MergeSetSegmentByteIterator& operator+=(ptrdiff_t diff)
		{
			mItemPtr += diff;
			return *this;
		}

		friend ptrdiff_t operator-(MergeSetSegmentByteIterator iter1,
			MergeSetSegmentByteIterator iter2) noexcept
		{
			return iter1.mItemPtr - iter2.mItemPtr;
		}

		Pointer operator->() const
		{
			return PtrCaster::ToBytePtr(mItemPtr);
		}

		friend bool operator==(MergeSetSegmentByteIterator iter1,
			MergeSetSegmentByteIterator iter2) noexcept
		{
			return iter1.mItemPtr == iter2.mItemPtr;
		}

	private:
		Item* mItemPtr;
	};

	template<typename TItem, typename TSettings>
	class MOMO_EMPTY_BASES MergeSetIterator
		: private VersionKeeper<TSettings>,
		public ForwardIteratorBase
	{
	protected:
		typedef TItem Item;
		typedef TSettings Settings;

		typedef internal::VersionKeeper<Settings> VersionKeeper;

		typedef MergeSetSegment<Item> Segment;

	public:
		typedef const Item& Reference;
		typedef const Item* Pointer;

		typedef MergeSetIterator ConstIterator;

	public:
		explicit MergeSetIterator() noexcept
			: mItemPtr(nullptr),
			mSegment(nullptr)
		{
		}

		//operator ConstIterator() const noexcept

		Pointer operator->() const
		{
			VersionKeeper::Check();
			MOMO_CHECK(mItemPtr != nullptr);
			return mItemPtr;
		}

		MergeSetIterator& operator++()
		{
			operator->();	// check
			++mItemPtr;
			pvMove();
			return *this;
		}

		using ForwardIteratorBase::operator++;

		friend bool operator==(MergeSetIterator iter1, MergeSetIterator iter2) noexcept
		{
			return iter1.mItemPtr == iter2.mItemPtr;
		}

	protected:
		explicit MergeSetIterator(const Segment* segment, Item* itemPtr,
			const size_t* version) noexcept
			: VersionKeeper(version),
			mItemPtr(itemPtr),
			mSegment(segment)
		{
			pvMove();
		}

		Item* ptGetItemPtr() const noexcept
		{
			return mItemPtr;
		}

		const Segment* ptGetSegment() const noexcept
		{
			return mSegment;
		}

		void ptCheck(const size_t* version, bool allowEmpty) const
		{
			VersionKeeper::Check(version, allowEmpty);
			MOMO_CHECK(allowEmpty || mItemPtr != nullptr);
		}

	private:
		void pvMove() noexcept
		{
			while (true)
			{
				if (mSegment->itemFlags == nullptr)
				{
					if (mItemPtr != mSegment->items + mSegment->count)
						return;
				}
				else
				{
					Item* segEnd = mSegment->items + mSegment->capacity;
					while (mItemPtr != segEnd && UIntMath<uint8_t>::GetBit(
						mSegment->itemFlags, UIntMath<>::Dist(mSegment->items, mItemPtr)))
					{
						++mItemPtr;
					}
					if (mItemPtr != segEnd)
						return;
				}
				if (mSegment->isLast)
					break;
				--mSegment;
				mItemPtr = mSegment->items;
			}
			*this = MergeSetIterator();
		}

	private:
		Item* mItemPtr;
		const Segment* mSegment;
	};

	template<typename TItem, typename TSettings>
	class MOMO_EMPTY_BASES MergeSetPosition : public MergeSetIterator<TItem, TSettings>
	{
	public:
		typedef MergeSetIterator<TItem, TSettings> Iterator;

		typedef MergeSetPosition ConstPosition;

	public:
		using Iterator::Iterator;

		MergeSetPosition(Iterator iter) noexcept
			: Iterator(iter)
		{
		}

	private:
		using Iterator::operator++;	//?
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
	1. Functions `Insert` receiving many items have basic exception safety.
	2. Function `Remove` receiving predicate has basic exception safety.
	3. Functions `MergeFrom` and `MergeTo` have basic exception safety.
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
	//typedef typename MergeTraits::BloomFilter BloomFilter;

	template<typename KeyArg>
	using IsValidKeyArg = MergeTraits::template IsValidKeyArg<KeyArg>;

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
	typedef internal::NestedArrayIntCap<0, Item*, MemManagerPtr> ItemPtrs;
	typedef internal::NestedArrayIntCap<0, ItemPtrCode, MemManagerPtr> ItemPtrCodes;

	typedef internal::UIntMath<> SMath;
	typedef internal::UIntMath<uint8_t> BMath;

	static const size_t hashCodeSize = SMath::Min(sizeof(size_t), sizeof(Item));

	struct ConstIteratorProxy : private ConstIterator
	{
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetSegment)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetItemPtr)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, Check)
	};

	struct IteratorProxy : public Iterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
	};

	struct PositionProxy : public Position
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Position)
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
		const Segment& backSegment = mSegments.GetBackItem();
		return IteratorProxy(&backSegment, backSegment.items, mCrew.GetVersion());
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

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	Position Find(const KeyArg& key) const
	{
		return pvFind(key);
	}

	bool ContainsKey(const Key& key) const
	{
		return !!pvFind(key);
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	bool ContainsKey(const KeyArg& key) const
	{
		return !!pvFind(key);
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

	Iterator Remove(ConstIterator iter)
		requires (MergeTraits::func == MergeTraitsFunc::hash)
	{
		auto itemRemover = [this] (Item& item)
			{ ItemTraits::Destroy(&GetMemManager(), item); };
		return pvRemove(iter, FastMovableFunctor(std::move(itemRemover)));
	}

	void Remove(ConstPosition pos)
		requires (MergeTraits::func == MergeTraitsFunc::hash)
	{
		Remove(static_cast<ConstIterator>(pos));
	}

	Iterator Remove(ConstIterator iter, ExtractedItem& extItem)
		requires (MergeTraits::func == MergeTraitsFunc::hash)
	{
		Iterator resIter;
		auto itemCreator = [this, iter, &resIter] (Item* newItem)
		{
			auto itemRemover = [this, newItem] (Item& item)
				{ ItemTraits::Relocate(&GetMemManager(), nullptr, item, newItem); };
			resIter = pvRemove(iter, FastMovableFunctor(std::move(itemRemover)));
		};
		extItem.Create(itemCreator);
		return resIter;
	}

	bool Remove(const Key& key)
		requires (MergeTraits::func == MergeTraitsFunc::hash)
	{
		Position pos = pvFind(key);
		if (!pos)
			return false;
		Remove(pos);
		return true;
	}

	template<internal::conceptObjectPredicate<Item> ItemFilter>
	size_t Remove(ItemFilter itemFilter)
		requires (MergeTraits::func == MergeTraitsFunc::hash)
	{
		size_t initCount = GetCount();
		Iterator iter = GetBegin();
		while (!!iter)
		{
			if (itemFilter(*iter))	//?
				iter = Remove(iter);
			else
				++iter;
		}
		return initCount - GetCount();
	}

	ExtractedItem Extract(ConstPosition pos)
		requires (MergeTraits::func == MergeTraitsFunc::hash)
	{
		return ExtractedItem(*this, static_cast<ConstIterator>(pos));
	}

	//template<typename KeyArg,
	//	bool extraCheck = true>
	//void ResetKey(ConstPosition pos, KeyArg&& keyArg)

	template<typename RSet>
	void MergeFrom(RSet&& srcSet)
	{
		srcSet.MergeTo(*this);
	}

	template<typename Set>
	requires std::is_same_v<ItemTraits, typename Set::ItemTraits>
	void MergeTo(Set& dstSet)
		requires (MergeTraits::func == MergeTraitsFunc::hash)
	{
		pvMergeTo(dstSet);
	}

	void MergeTo(MergeSetCore& dstMergeSet)
		requires (MergeTraits::func == MergeTraitsFunc::hash)
	{
		if (this == &dstMergeSet)
			return;
		pvMergeTo(dstMergeSet);
	}

	void CheckIterator(ConstIterator iter, bool allowEmpty = true) const
	{
		ConstIteratorProxy::Check(iter, mCrew.GetVersion(), allowEmpty);
	}

private:
	Item* pvAllocateSegmentItems(size_t capacity)
	{
		if (capacity > internal::UIntConst::maxSize / sizeof(Item))
			MOMO_THROW(std::bad_array_new_length());
		static_assert(internal::ObjectAlignmenter<Item>::Check(ItemTraits::alignment));
		return MemManagerProxy::template Allocate<Item>(GetMemManager(),
			capacity * sizeof(Item));
	}

	void pvDeallocateSegmentItems(Item* segItems, size_t capacity) noexcept
	{
		MemManagerProxy::Deallocate(GetMemManager(), segItems, capacity * sizeof(Item));
	}

	uint8_t* pvAllocateSegmentFlags(size_t capacity)
	{
		size_t length = SMath::Ceil(capacity, 8);
		uint8_t* segItemFlags = MemManagerProxy::template Allocate<uint8_t>(GetMemManager(), length);
		std::fill_n(segItemFlags, length, uint8_t{0});
		return segItemFlags;
	}

	void pvDeallocateSegmentFlags(uint8_t* segItemFlags, size_t capacity) noexcept
	{
		MemManagerProxy::Deallocate(GetMemManager(), segItemFlags, SMath::Ceil(capacity, 8));
	}

	void pvDeallocateSegment(Segment& segment) noexcept
	{
		MOMO_ASSERT(segment.count == 0);
		if (segment.items != nullptr)
			pvDeallocateSegmentItems(segment.items, segment.capacity);
		if (MergeTraits::func == MergeTraitsFunc::hash && segment.itemFlags != nullptr)
			pvDeallocateSegmentFlags(segment.itemFlags, segment.capacity);
		segment.items = nullptr;
		segment.itemFlags = nullptr;
		segment.capacity = 0;
	}

	void pvDestroy() noexcept
	{
		MemManager& memManager = GetMemManager();
		size_t segCount = mSegments.GetCount();
		for (size_t s = 0; s < segCount; ++s)
		{
			Segment& segment = mSegments[s];
			if (MergeTraits::func == MergeTraitsFunc::hash && segment.itemFlags != nullptr)
			{
				for (size_t i = 0; i < segment.capacity; ++i)
				{
					if (BMath::GetBit(segment.itemFlags, i))
						continue;
					ItemTraits::Destroy(&memManager, segment.items[i]);
				}
			}
			else
			{
				for (size_t i = 0; i < segment.count; ++i)
					ItemTraits::Destroy(&memManager, segment.items[i]);
			}
			segment.count = 0;
			pvDeallocateSegment(segment);
		}
		mSegments.Clear(true);
	}

	Position pvMakePosition(const Segment& segment, Item* itemPtr) const noexcept
	{
		return PositionProxy(&segment, itemPtr, mCrew.GetVersion());
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

	static std::byte* pvGetHashCodePtr(size_t* hashCode) noexcept
	{
		std::byte* ptr = internal::PtrCaster::ToBytePtr(hashCode);
		if constexpr (std::endian::native == std::endian::little)
			ptr += sizeof(size_t) - hashCodeSize;
		return ptr;
	}

	template<typename KeyArg>
	size_t pvGetHashCode(const KeyArg& key) const
	{
		size_t hashCode = GetMergeTraits().GetHashCode(key);
		size_t resHashCode = 0;
		internal::MemCopyer::CopyBuffer<hashCodeSize>(
			pvGetHashCodePtr(&hashCode), pvGetHashCodePtr(&resHashCode));
		return resHashCode;
	}

	template<typename KeyArg>
	Position pvFind(const KeyArg& key) const
	{
		const MergeTraits& mergeTraits = GetMergeTraits();
		size_t hashCode = 0;
		if constexpr (MergeTraits::func == MergeTraitsFunc::hash)
			hashCode = pvGetHashCode(key);
		auto itemPred = [&mergeTraits, &key] (const Item& item)
			{ return mergeTraits.IsEqual(ItemTraits::GetKey(item), key); };
		for (size_t s = mSegments.GetCount(); s > 1; --s)
		{
			const Segment& segment = mSegments[s - 1];
			if (segment.count == 0)
				continue;
			Item* segItems = segment.items;
			if constexpr (MergeTraits::func == MergeTraitsFunc::hash)
			{
				uint8_t* segItemFlags = segment.itemFlags;
				auto hasher = [this, segItemFlags, segItems] (const std::byte& byteItem)
				{
					if (segItemFlags != nullptr)
					{
						size_t segItemIndex = SMath::Dist(segItems,
							internal::PtrCaster::FromBytePtr<Item>(&byteItem));
						if (BMath::GetBit(segItemFlags, segItemIndex))
						{
							size_t hashCode = 0;
							internal::MemCopyer::CopyBuffer<hashCodeSize>(
								&byteItem, pvGetHashCodePtr(&hashCode));
							return hashCode;
						}
					}
					return pvGetHashCode(ItemTraits::GetKey(
						*internal::PtrCaster::FromBytePtr<Item, true, true>(&byteItem)));
				};
				auto equalComp = [&mergeTraits, segItemFlags, segItems]
					(const std::byte& item1, auto item2)
				{
					if constexpr (std::is_same_v<decltype(item2), const KeyArg*>)
					{
						if (segItemFlags != nullptr)
						{
							size_t segItemIndex = SMath::Dist(segItems,
								internal::PtrCaster::FromBytePtr<Item>(&item1));
							if (BMath::GetBit(segItemFlags, segItemIndex))
								return false;
						}
						return mergeTraits.IsEqual(
							ItemTraits::GetKey(*internal::PtrCaster::FromBytePtr<Item, true, true>(&item1)),
							*item2);
					}
					else
					{
						static_assert(std::is_same_v<decltype(item2), std::byte>);
						return false;
					}
				};
				internal::MergeSetSegmentByteIterator<Item> segByteItems(segItems);
				auto findRes = HashSorter::Find(segByteItems,
					segItemFlags == nullptr ? segment.count : segment.capacity,
					std::addressof(key), hashCode, hasher, equalComp);
				if (findRes.found)
					return pvMakePosition(segment, segItems + (findRes.iterator - segByteItems));
			}
			else
			{
				auto lessComp = [&mergeTraits] (const Item& item1, const KeyArg& key2)
					{ return mergeTraits.IsLess(ItemTraits::GetKey(item1), key2); };
				Item* itemPtr = std::lower_bound(segItems,
					segItems + segment.count - 1, key, FastCopyableFunctor(lessComp));
				if (itemPred(*itemPtr))
					return pvMakePosition(segment, itemPtr);
			}
		}
		if (!mSegments.IsEmpty())
		{
			const Segment& segment0 = mSegments[0];
			Item* segItems = segment0.items;
			uint8_t* segItemFlags = segment0.itemFlags;
			if (MergeTraits::func == MergeTraitsFunc::hash && segItemFlags != nullptr)
			{
				for (size_t i = 0; i < segment0.capacity; ++i)
				{
					if (!BMath::GetBit(segItemFlags, i) && itemPred(segItems[i]))
						return pvMakePosition(segment0, segItems + i);
				}
			}
			else
			{
				for (size_t i = 0; i < segment0.count; ++i)
				{
					if (itemPred(segItems[i]))
						return pvMakePosition(segment0, segItems + i);
				}
			}
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
		Position resPos = pvMakePosition(mSegments[0], itemPtr);
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
		if (segment0.items == nullptr)
		{
			size_t capacity0 = GetMergeTraits().GetSegmentItemCount(0);
			MOMO_ASSERT(capacity0 > 0);
			segment0.items = pvAllocateSegmentItems(capacity0);
			segment0.capacity = capacity0;
		}
		if (segment0.count < segment0.capacity)
		{
			uint8_t* segItemFlags = segment0.itemFlags;
			size_t segItemIndex = segment0.count;
			if (MergeTraits::func == MergeTraitsFunc::hash && segItemFlags != nullptr)
			{
				segItemIndex = 0;
				while (!BMath::GetBit(segItemFlags, segItemIndex))
					++segItemIndex;
			}
			std::move(itemCreator)(segment0.items + segItemIndex);
			++segment0.count;
			if (MergeTraits::func == MergeTraitsFunc::hash && segItemFlags != nullptr)
				BMath::SetBit(segItemFlags, segItemIndex, false);
			return segment0.items + segItemIndex;
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
		Item* items0 = mSegments[0].items;
		size_t itemCount0 = mSegments[0].count;
		size_t relItemCount = itemCount0;
		size_t segIndex = 1;
		while (true)
		{
			if (segIndex < mSegments.GetCount())
				relItemCount += mSegments[segIndex].count;
			if (relItemCount <= mergeTraits.GetSegmentItemCount(segIndex))
				break;
			++segIndex;
		}
		if (mSegments.GetCount() <= segIndex)
			mSegments.SetCount(segIndex + 1, Segment());
		size_t capacity0 = mergeTraits.GetSegmentItemCount(0);
		Item* newItems0 = pvAllocateSegmentItems(capacity0);
		internal::Finalizer fin0(&MergeSetCore::pvDeallocateSegmentItems, *this, newItems0, capacity0);
		Item* newItems = pvAllocateSegmentItems(relItemCount);
		internal::Finalizer fin(&MergeSetCore::pvDeallocateSegmentItems, *this, newItems, relItemCount);
		if constexpr (MergeTraits::func == MergeTraitsFunc::hash)
		{
			ItemPtrCodes itemPtrCodes(relItemCount, MemManagerPtr(memManager));
			for (size_t i = 0; i < itemCount0; ++i)
			{
				Item* itemPtr = items0 + i;
				itemPtrCodes[relItemCount - itemCount0 + i] =
					{ itemPtr, pvGetHashCode(ItemTraits::GetKey(*itemPtr)) };
			}
			auto lessComp = [] (ItemPtrCode itemPtrCode1, ItemPtrCode itemPtrCode2) noexcept
				{ return itemPtrCode1.hashCode < itemPtrCode2.hashCode; };
			pvSortPtrs(&itemPtrCodes[relItemCount - itemCount0], itemCount0, lessComp);
			size_t curItemCount = itemCount0;
			for (size_t s = 1; s <= segIndex; ++s)
			{
				size_t segItemCount = mSegments[s].count;
				if (segItemCount == 0)
					continue;
				curItemCount += segItemCount;
				pvMergePtrCodes(mSegments[s].items, mSegments[s].itemFlags, segItemCount,
					mSegments[s].capacity, &itemPtrCodes[relItemCount - curItemCount], curItemCount);
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
			for (size_t i = 0; i < itemCount0; ++i)
				pvRelocate(items0[i], newItems + relItemCount - itemCount0 + i);
			pvSortRelocate(newItems + relItemCount - itemCount0, itemCount0);
			size_t curItemCount = itemCount0;
			for (size_t s = 1; s <= segIndex; ++s)
			{
				size_t segItemCount = mSegments[s].count;
				if (segItemCount == 0)
					continue;
				curItemCount += segItemCount;
				pvMergeRelocate(mSegments[s].items, segItemCount,
					newItems + relItemCount - curItemCount, curItemCount);
			}
		}
		else
		{
			ItemPtrs itemPtrs(relItemCount, MemManagerPtr(memManager));
			for (size_t i = 0; i < itemCount0; ++i)
				itemPtrs[relItemCount - itemCount0 + i] = items0 + i;
			auto lessComp = [&mergeTraits] (Item* itemPtr1, Item* itemPtr2)
			{
				return mergeTraits.IsLess(ItemTraits::GetKey(*itemPtr1), ItemTraits::GetKey(*itemPtr2));
			};
			pvSortPtrs(&itemPtrs[relItemCount - itemCount0], itemCount0, lessComp);
			size_t curItemCount = itemCount0;
			for (size_t s = 1; s <= segIndex; ++s)
			{
				size_t segItemCount = mSegments[s].count;
				if (segItemCount == 0)
					continue;
				curItemCount += segItemCount;
				pvMergePtrs(mSegments[s].items, segItemCount,
					&itemPtrs[relItemCount - curItemCount], curItemCount);
			}
			internal::IncIterator srcIter = [iter = itemPtrs.GetItems()] () mutable noexcept
				{ return *iter++; };
			ItemTraits::RelocateCreate(memManager,
				srcIter, newItems, relItemCount, std::move(itemCreator), newItems0);
		}
		for (size_t s = 0; s <= segIndex; ++s)
		{
			mSegments[s].count = 0;
			pvDeallocateSegment(mSegments[s]);
		}
		mSegments[segIndex].items = newItems;
		mSegments[segIndex].capacity = relItemCount;
		mSegments[segIndex].count = relItemCount;
		fin.Detach();
		mSegments[0].items = newItems0;
		mSegments[0].capacity = capacity0;
		mSegments[0].count = 1;
		fin0.Detach();
		return newItems0;
	}

	template<typename ItemPtr, typename LessComparer>
	void pvSortPtrs(ItemPtr* itemPtrs, size_t count, const LessComparer& lessComp)
	{
		for (size_t i = 1; i < count; ++i)
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

	void pvSortRelocate(Item* items, size_t count) noexcept
	{
		const MergeTraits& mergeTraits = GetMergeTraits();
		for (size_t i = 1; i < count; ++i)
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
		
	void pvMergePtrCodes(Item* srcItems1, uint8_t* srcItemFlags1, size_t srcCount1, size_t srcCapacity1,
		ItemPtrCode* dstItemPtrCodes, size_t dstCount)
	{
		ItemPtrCode* srcItemPtrCodes2 = dstItemPtrCodes + srcCount1;
		size_t srcCount2 = dstCount - srcCount1;
		size_t srcIndex1 = 0;
		size_t srcIndex2 = 0;
		size_t dstIndex = 0;
		while (true)
		{
			if (srcItemFlags1 == nullptr)
			{
				if (srcIndex1 >= srcCount1)
					break;
			}
			else
			{
				while (srcIndex1 < srcCapacity1 && BMath::GetBit(srcItemFlags1, srcIndex1))
					++srcIndex1;
				if (srcIndex1 >= srcCapacity1)
					break;
			}
			size_t srcHashCode1 = pvGetHashCode(ItemTraits::GetKey(srcItems1[srcIndex1]));
			while (srcIndex2 < srcCount2 && srcItemPtrCodes2[srcIndex2].hashCode < srcHashCode1)
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

	void pvMergeRelocate(Item* srcItems1, size_t srcCount1, Item* dstItems, size_t dstCount)
	{
		const MergeTraits& mergeTraits = GetMergeTraits();
		Item* srcItems2 = dstItems + srcCount1;
		size_t srcCount2 = dstCount - srcCount1;
		size_t srcIndex1 = 0;
		size_t srcIndex2 = 0;
		size_t dstIndex = 0;
		while (srcIndex1 < srcCount1)
		{
			if (srcIndex2 < srcCount2 && mergeTraits.IsLess(
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

	void pvMergePtrs(Item* srcItems1, size_t srcCount1, Item** dstItemPtrs, size_t dstCount)
	{
		const MergeTraits& mergeTraits = GetMergeTraits();
		Item** srcItemPtrs2 = dstItemPtrs + srcCount1;
		size_t srcCount2 = dstCount - srcCount1;
		size_t srcIndex1 = 0;
		size_t srcIndex2 = 0;
		size_t dstIndex = 0;
		while (srcIndex1 < srcCount1)
		{
			if (srcIndex2 < srcCount2 && mergeTraits.IsLess(
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

	void pvRelocate(Item& srcItem, Item* dstItem) noexcept(ItemTraits::isNothrowRelocatable)
	{
		MemManager& memManager = GetMemManager();
		ItemTraits::Relocate(&memManager, &memManager, srcItem, dstItem);
	}

	template<internal::conceptObjectRemover<Item> ItemRemover>
	Iterator pvRemove(ConstIterator iter, FastMovableFunctor<ItemRemover> itemRemover)
		requires (MergeTraits::func == MergeTraitsFunc::hash)
	{
		CheckIterator(iter, false);
		Segment& segment = mSegments[SMath::Dist(&mSegments[0], ConstIteratorProxy::GetSegment(iter))];
		Item* itemPtr = ConstIteratorProxy::GetItemPtr(iter);
		bool isLastSeg = segment.isLast;
		bool dealloc = (segment.count == 1 && !isLastSeg);
		bool useFlags = !dealloc
			&& !(isLastSeg && segment.itemFlags == nullptr && itemPtr == segment.items + segment.count - 1);
		size_t hashCode = (useFlags && !isLastSeg) ? pvGetHashCode(ItemTraits::GetKey(*itemPtr)) : 0;
		if (useFlags && segment.itemFlags == nullptr)
		{
			size_t segCapacity = segment.capacity;
			segment.itemFlags = pvAllocateSegmentFlags(segCapacity);
			MOMO_ASSERT(segment.count == segCapacity || isLastSeg);
			for (size_t i = segment.count; i < segCapacity; ++i)
				BMath::SetBit(segment.itemFlags, i);
		}
		std::move(itemRemover)(*itemPtr);
		--segment.count;
		if (useFlags)
		{
			BMath::SetBit(segment.itemFlags, SMath::Dist(segment.items, itemPtr));
			if (!isLastSeg)
				internal::MemCopyer::CopyBuffer<hashCodeSize>(pvGetHashCodePtr(&hashCode), itemPtr);
		}
		if (dealloc)
		{
			pvDeallocateSegment(segment);
			itemPtr = nullptr;
		}
		--mCount;
		mCrew.IncVersion();
		return IteratorProxy(&segment, itemPtr, mCrew.GetVersion());
	}

	template<typename Set>
	void pvMergeTo(Set& dstSet)
		requires (MergeTraits::func == MergeTraitsFunc::hash)
	{
		MemManager& memManager = GetMemManager();
		MemManager& dstMemManager = dstSet.GetMemManager();
		Iterator iter = GetBegin();
		while (!!iter)
		{
			auto itemCreator = [this, &memManager, &dstMemManager, &iter] (Item* newItem)
			{
				auto itemRemover = [&memManager, &dstMemManager, newItem] (Item& item)
					{ ItemTraits::Relocate(&memManager, &dstMemManager, item, newItem); };
				iter = pvRemove(iter, FastMovableFunctor(std::move(itemRemover)));
			};
			if (!dstSet.InsertCrt(ItemTraits::GetKey(*iter), std::move(itemCreator)).inserted)
				++iter;
		}
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
	template<typename I>
	struct iterator_traits<momo::internal::MergeSetSegmentByteIterator<I>>
		: public momo::internal::IteratorTraitsStd<momo::internal::MergeSetSegmentByteIterator<I>,
			random_access_iterator_tag>
	{
	};

	template<typename I, typename S>
	struct iterator_traits<momo::internal::MergeSetIterator<I, S>>
		: public momo::internal::IteratorTraitsStd<momo::internal::MergeSetIterator<I, S>,
			forward_iterator_tag>
	{
	};
} // namespace std

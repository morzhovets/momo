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
    class MergeSet
    class MergeSetHash

\**********************************************************/

#pragma once

#include "MergeTraits.h"
#include "SetUtility.h"
#include "HashSorter.h"

namespace momo
{

namespace internal
{
	template<typename TItem, typename TSettings>
	class MergeSetPosition : private VersionKeeper<TSettings>
	{
	protected:
		typedef TItem Item;
		typedef TSettings Settings;

		typedef internal::VersionKeeper<Settings> VersionKeeper;

	public:
		typedef const Item& Reference;
		typedef const Item* Pointer;

		typedef MergeSetPosition ConstPosition;

		//typedef ... Iterator;

	public:
		explicit MergeSetPosition() noexcept
			: mItemPtr(nullptr)
		{
		}

		//operator ConstPosition() const noexcept

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

		MOMO_MORE_POSITION_OPERATORS(MergeSetPosition)

	protected:
		explicit MergeSetPosition(const Item& item, const size_t* version) noexcept
			: VersionKeeper(version),
			mItemPtr(std::addressof(item))
		{
		}

	private:
		const Item* mItemPtr;
	};

	template<typename TMergeArrayIterator, typename TSettings>
	class MergeSetIterator : private VersionKeeper<TSettings>, private TMergeArrayIterator
	{
	protected:
		typedef TMergeArrayIterator MergeArrayIterator;
		typedef TSettings Settings;

		typedef internal::VersionKeeper<Settings> VersionKeeper;

	public:
		typedef typename MergeArrayIterator::Reference Reference;
		typedef typename MergeArrayIterator::Pointer Pointer;

		typedef MergeSetIterator ConstIterator;

	public:
		explicit MergeSetIterator() noexcept = default;

		//operator ConstIterator() const noexcept

		MergeSetIterator& operator++()
		{
			VersionKeeper::Check();
			const auto* array = MergeArrayIterator::ptGetArray();
			MOMO_CHECK(array != nullptr);
			if (MergeArrayIterator::ptGetIndex() + 1 < array->GetCount())
				MergeArrayIterator::operator++();
			else
				*this = MergeSetIterator();
			return *this;
		}

		Pointer operator->() const
		{
			VersionKeeper::Check();
			MOMO_CHECK(*this != MergeSetIterator());
			return MergeArrayIterator::operator->();
		}

		friend bool operator==(MergeSetIterator iter1, MergeSetIterator iter2) noexcept
		{
			return static_cast<MergeArrayIterator>(iter1) == static_cast<MergeArrayIterator>(iter2);
		}

		MOMO_MORE_FORWARD_ITERATOR_OPERATORS(MergeSetIterator)

	protected:
		explicit MergeSetIterator(MergeArrayIterator mergeArrayIterator,
			const size_t* version) noexcept
			: VersionKeeper(version),
			MergeArrayIterator(mergeArrayIterator)
		{
		}
	};

	template<typename TMergeSetCrew>
	class MergeSetNestedArrayMemManager : private TMergeSetCrew,
		public MemManagerPtr<typename TMergeSetCrew::MemManager>
	{
	public:
		typedef TMergeSetCrew MergeSetCrew;

		typedef internal::MemManagerPtr<typename MergeSetCrew::MemManager> MemManagerPtr;

	public:
		explicit MergeSetNestedArrayMemManager(MergeSetCrew&& mergeSetCrew) noexcept
			: MergeSetCrew(std::move(mergeSetCrew)),
			MemManagerPtr(GetMergeSetCrew().GetMemManager())
		{
		}

		MergeSetNestedArrayMemManager(MergeSetNestedArrayMemManager&& arrayMemManager) noexcept
			: MergeSetCrew(std::move(arrayMemManager.GetMergeSetCrew())),
			MemManagerPtr(GetMergeSetCrew().GetMemManager())
		{
		}

		MergeSetNestedArrayMemManager(const MergeSetNestedArrayMemManager&) = delete;

		~MergeSetNestedArrayMemManager() noexcept = default;

		MergeSetNestedArrayMemManager& operator=(const MergeSetNestedArrayMemManager&) = delete;

		const MergeSetCrew& GetMergeSetCrew() const noexcept
		{
			return *this;
		}

		MergeSetCrew& GetMergeSetCrew() noexcept
		{
			return *this;
		}
	};

	template<typename TMergeSetItemTraits, typename TMemManager>
	class MergeSetNestedArrayItemTraits
	{
	protected:
		typedef TMergeSetItemTraits MergeSetItemTraits;

	public:
		typedef TMemManager MemManager;

		typedef typename MergeSetItemTraits::Item Item;

		static const size_t alignment = MergeSetItemTraits::alignment;

	private:
		typedef typename MergeSetItemTraits::Key Key;

		typedef typename MemManager::MemManagerPtr MemManagerPtr;
		typedef typename MemManager::BaseMemManager SetMemManager;
		typedef typename MemManager::MergeSetCrew::ContainerTraits MergeTraits;

		static const size_t initialItemCount =
			size_t{1} << MergeTraits::MergeArraySettings::logInitialItemCount;

		struct ItemPtrHash
		{
			Item* ptr;
			size_t hash;
		};

		typedef NestedArrayIntCap<initialItemCount <= 16 ? 32 : 0, Item*, MemManagerPtr> ItemPtrs;
		typedef NestedArrayIntCap<initialItemCount <= 16 ? 32 : 0,	//?
			ItemPtrHash, MemManagerPtr> ItemPtrHashes;

	public:
		template<conceptIncIterator<Item> Iterator>
		static void Destroy(MemManager& memManager, Iterator begin, size_t count) noexcept
		{
			SetMemManager* setMemManager = &memManager.GetBaseMemManager();
			Iterator iter = begin;
			for (size_t i = 0; i < count; ++i)
				MergeSetItemTraits::Destroy(setMemManager, *iter++);
		}

		template<conceptIncIterator<Item> SrcIterator, conceptIncIterator<Item> DstIterator,
			conceptObjectCreator<Item> ItemCreator>
		requires conceptRandomIterator17<SrcIterator> && std::is_same_v<DstIterator, Item*>
		static void RelocateCreate(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
			size_t count, FastMovableFunctor<ItemCreator> itemCreator, Item* newItem)
		{
			MOMO_ASSERT(std::has_single_bit(count) && count > initialItemCount);
			const MergeTraits& mergeTraits = memManager.GetMergeSetCrew().GetContainerTraits();
			Item* srcItems1 = pvNextAddr(srcBegin, count - initialItemCount);
			Item* srcItems2 = pvNextAddr(srcBegin, count - 2 * initialItemCount);
			if constexpr (MergeTraits::func == MergeTraitsFunc::hash)
			{
				ItemPtrHashes itemPtrHashes(count, memManager);
				for (size_t i = 0; i < initialItemCount; ++i)
				{
					itemPtrHashes[count - initialItemCount + i] = { srcItems1 + i,
						mergeTraits.GetHashCode(MergeSetItemTraits::GetKey(srcItems1[i])) };
					itemPtrHashes[count - 2 * initialItemCount + i] = { srcItems2 + i,
						mergeTraits.GetHashCode(MergeSetItemTraits::GetKey(srcItems2[i])) };
				}
				auto lessFunc = [] (ItemPtrHash itemPtrHash1, ItemPtrHash itemPtrHash2) noexcept
					{ return itemPtrHash1.hash < itemPtrHash2.hash; };
				pvSortPtrs(&itemPtrHashes[count - 2 * initialItemCount], lessFunc);
				for (size_t index = 2 * initialItemCount; index < count; index *= 2)
				{
					Item* srcItems = pvNextAddr(srcBegin, count - 2 * index);
					pvMergePtrHashes(mergeTraits, srcItems, &itemPtrHashes[count - 2 * index], index);
				}
				IncIterator srcIter = [iter = itemPtrHashes.GetItems()] () mutable noexcept
					{ return (iter++)->ptr; };
				MergeSetItemTraits::RelocateCreate(memManager.GetBaseMemManager(),
					srcIter, dstBegin, count, std::move(itemCreator), newItem);
			}
			else if constexpr (MergeTraits::func == MergeTraitsFunc::lessNothrow
				&& MergeSetItemTraits::isNothrowRelocatable)
			{
				std::move(itemCreator)(newItem);
				for (size_t i = 0; i < initialItemCount; ++i)
				{
					pvRelocate(memManager, srcItems1[i], dstBegin + count - initialItemCount + i);
					pvRelocate(memManager, srcItems2[i], dstBegin + count - 2 * initialItemCount + i);
				}
				pvSortRelocate(memManager, mergeTraits, dstBegin + count - 2 * initialItemCount);
				for (size_t index = 2 * initialItemCount; index < count; index *= 2)
				{
					Item* srcItems = pvNextAddr(srcBegin, count - 2 * index);
					pvMergeRelocate(memManager, mergeTraits, srcItems,
						dstBegin + count - 2 * index, index);
				}
			}
			else
			{
				ItemPtrs itemPtrs(count, memManager);
				for (size_t i = 0; i < initialItemCount; ++i)
				{
					itemPtrs[count - initialItemCount + i] = srcItems1 + i;
					itemPtrs[count - 2 * initialItemCount + i] = srcItems2 + i;
				}
				auto lessFunc = [&mergeTraits] (Item* itemPtr1, Item* itemPtr2)
				{
					return mergeTraits.IsLess(MergeSetItemTraits::GetKey(*itemPtr1),
						MergeSetItemTraits::GetKey(*itemPtr2));
				};
				pvSortPtrs(&itemPtrs[count - 2 * initialItemCount], lessFunc);
				for (size_t index = 2 * initialItemCount; index < count; index *= 2)
				{
					Item* srcItems = pvNextAddr(srcBegin, count - 2 * index);
					pvMergePtrs(mergeTraits, srcItems, &itemPtrs[count - 2 * index], index);
				}
				IncIterator srcIter = [iter = itemPtrs.GetItems()] () mutable noexcept
					{ return *iter++; };
				MergeSetItemTraits::RelocateCreate(memManager.GetBaseMemManager(),
					srcIter, dstBegin, count, std::move(itemCreator), newItem);
			}
		}

	private:
		template<conceptIncIterator<Item> Iterator>
		requires conceptRandomIterator17<Iterator>
		static Item* pvNextAddr(Iterator iter, size_t dist)
		{
			return std::to_address(UIntMath<>::Next(iter, dist));
		}

		static void pvRelocate(MemManager& memManager, Item& srcItem, Item* dstItem)
			noexcept(MergeSetItemTraits::isNothrowRelocatable)
		{
			SetMemManager* setMemManager = &memManager.GetBaseMemManager();
			MergeSetItemTraits::Relocate(setMemManager, setMemManager, srcItem, dstItem);
		}

		static void pvSortRelocate(MemManager& memManager, const MergeTraits& mergeTraits, Item* items)
		{
			for (size_t i = 1; i < 2 * initialItemCount; ++i)
			{
				ObjectBuffer<Item, alignment> itemBuffer;
				pvRelocate(memManager, items[i], &itemBuffer);
				const Key& key = MergeSetItemTraits::GetKey(*&itemBuffer);
				size_t j = i;
				for (; j > 0; --j)
				{
					if (!mergeTraits.IsLess(key, MergeSetItemTraits::GetKey(items[j - 1])))
						break;
					pvRelocate(memManager, items[j - 1], items + j);
				}
				pvRelocate(memManager, *&itemBuffer, items + j);
			}
		}
		
		template<typename ItemPtr, typename LessFunc>
		static void pvSortPtrs(ItemPtr* itemPtrs, const LessFunc& lessFunc)
		{
			for (size_t i = 1; i < 2 * initialItemCount; ++i)
			{
				ItemPtr itemPtr = itemPtrs[i];
				size_t j = i;
				for (; j > 0; --j)
				{
					if (!lessFunc(itemPtr, itemPtrs[j - 1]))
						break;
					itemPtrs[j] = itemPtrs[j - 1];
				}
				itemPtrs[j] = itemPtr;
			}
		}

		static void pvMergeRelocate(MemManager& memManager, const MergeTraits& mergeTraits,
			Item* srcItems1, Item* dstItems, size_t count)
		{
			Item* srcItems2 = dstItems + count;
			size_t srcIndex1 = 0;
			size_t srcIndex2 = 0;
			size_t dstIndex = 0;
			while (srcIndex1 < count)
			{
				if (srcIndex2 < count && mergeTraits.IsLess(
					MergeSetItemTraits::GetKey(srcItems2[srcIndex2]),
					MergeSetItemTraits::GetKey(srcItems1[srcIndex1])))
				{
					pvRelocate(memManager, srcItems2[srcIndex2], dstItems + dstIndex);
					++srcIndex2;
				}
				else
				{
					pvRelocate(memManager, srcItems1[srcIndex1], dstItems + dstIndex);
					++srcIndex1;
				}
				++dstIndex;
			}
		}

		static void pvMergePtrs(const MergeTraits& mergeTraits, Item* srcItems1,
			Item** dstItemPtrs, size_t count)
		{
			Item** srcItemPtrs2 = dstItemPtrs + count;
			size_t srcIndex1 = 0;
			size_t srcIndex2 = 0;
			size_t dstIndex = 0;
			while (srcIndex1 < count)
			{
				if (srcIndex2 < count && mergeTraits.IsLess(
					MergeSetItemTraits::GetKey(*srcItemPtrs2[srcIndex2]),
					MergeSetItemTraits::GetKey(srcItems1[srcIndex1])))
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

		static void pvMergePtrHashes(const MergeTraits& mergeTraits, Item* srcItems1,
			ItemPtrHash* dstItemPtrHashes, size_t count)
		{
			ItemPtrHash* srcItemPtrHashes2 = dstItemPtrHashes + count;
			size_t srcIndex1 = 0;
			size_t srcIndex2 = 0;
			size_t dstIndex = 0;
			while (srcIndex1 < count)
			{
				size_t srcHash1 =
					mergeTraits.GetHashCode(MergeSetItemTraits::GetKey(srcItems1[srcIndex1]));
				while (srcIndex2 < count && srcItemPtrHashes2[srcIndex2].hash < srcHash1)
				{
					dstItemPtrHashes[dstIndex] = srcItemPtrHashes2[srcIndex2];
					++srcIndex2;
					++dstIndex;
				}
				dstItemPtrHashes[dstIndex] = { srcItems1 + srcIndex1, srcHash1 };
				++srcIndex1;
				++dstIndex;
			}
		}
	};
}

template<conceptObject TKey, conceptMemManager TMemManager>
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
};

/*!
	All `MergeSet` functions and constructors have strong exception safety,
	but not the following cases:
	 - Functions `Insert` receiving many items have basic exception safety.

	Swap and move operations invalidate all container iterators.
*/

template<conceptObject TKey,
	conceptMergeTraits<TKey> TMergeTraits = MergeTraits<TKey>,
	conceptMemManager TMemManager = MemManagerDefault,
	conceptSetItemTraits<TKey, TMemManager> TItemTraits = MergeSetItemTraits<TKey, TMemManager>,
	typename TSettings = MergeSetSettings>
class MergeSet
{
public:
	typedef TKey Key;
	typedef TMergeTraits MergeTraits;
	typedef TMemManager MemManager;
	typedef TItemTraits ItemTraits;
	typedef TSettings Settings;
	typedef typename ItemTraits::Item Item;

private:
	typedef internal::SetCrew<MergeTraits, MemManager, Settings::checkVersion> Crew;

	typedef internal::MergeSetNestedArrayMemManager<Crew> MergeArrayMemManager;
	typedef internal::MergeSetNestedArrayItemTraits<ItemTraits,
		MergeArrayMemManager> MergeArrayItemTraits;
	typedef internal::NestedArraySettings<
		typename MergeTraits::MergeArraySettings> MergeArraySettings;	//?

	typedef momo::MergeArray<Item, MergeArrayMemManager, MergeArrayItemTraits,
		MergeArraySettings> MergeArray;

	typedef typename MergeTraits::BloomFilter BloomFilter;

	static const size_t initialItemCount = size_t{1} << MergeArraySettings::logInitialItemCount;

public:
	typedef internal::MergeSetIterator<typename MergeArray::ConstIterator, Settings> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::MergeSetPosition<Item, Settings> Position;
	typedef typename Position::ConstPosition ConstPosition;

	typedef internal::InsertResult<Position> InsertResult;

private:
	template<typename... ItemArgs>
	using Creator = typename ItemTraits::template Creator<ItemArgs...>;

	struct PositionProxy : public Position
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Position)
	};

	struct IteratorProxy : public Iterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
	};

public:
	MergeSet()
		: MergeSet(MergeTraits())
	{
	}

	explicit MergeSet(const MergeTraits& mergeTraits, MemManager memManager = MemManager())
		: mMergeArray(MergeArrayMemManager(Crew(mergeTraits, std::move(memManager))))
	{
	}

	MergeSet(std::initializer_list<Item> items)
		: MergeSet(items, MergeTraits())
	{
	}

	explicit MergeSet(std::initializer_list<Item> items, const MergeTraits& mergeTraits,
		MemManager memManager = MemManager())
		: MergeSet(mergeTraits, std::move(memManager))
	{
		try
		{
			Insert(items);
		}
		catch (...)
		{
			pvFilterClear();
			throw;
		}
	}

	MergeSet(MergeSet&& mergeSet) noexcept
		: mMergeArray(std::move(mergeSet.mMergeArray)),
		mBloomFilter(std::move(mergeSet.mBloomFilter))
	{
	}

	MergeSet(const MergeSet& mergeSet)
		: MergeSet(mergeSet, MemManager(mergeSet.GetMemManager()))
	{
	}

	explicit MergeSet(const MergeSet& mergeSet, MemManager memManager)
		: mMergeArray(MergeArray::CreateCap(mergeSet.GetCount(),
			MergeArrayMemManager(Crew(mergeSet.GetMergeTraits(), std::move(memManager)))))
	{
		MemManager& thisMemManager = GetMemManager();
		for (const Item& item : mergeSet.mMergeArray)
			mMergeArray.AddBackNogrowCrt(Creator<const Item&>(thisMemManager, item));
		try
		{
			mBloomFilter.Init(thisMemManager, pvGetFilterLogMaxCount(), mergeSet.mBloomFilter);
		}
		catch (...)
		{
		}
	}

	~MergeSet() noexcept
	{
		pvFilterClear();
	}

	MergeSet& operator=(MergeSet&& mergeSet) noexcept
	{
		MergeSet(std::move(mergeSet)).Swap(*this);
		return *this;
	}

	MergeSet& operator=(const MergeSet& mergeSet)
	{
		if (this != &mergeSet)
			MergeSet(mergeSet).Swap(*this);
		return *this;
	}

	void Swap(MergeSet& mergeSet) noexcept
	{
		mMergeArray.Swap(mergeSet.mMergeArray);
		mBloomFilter.Swap(mergeSet.mBloomFilter);
	}

	Iterator GetBegin() const noexcept
	{
		if (IsEmpty())
			return Iterator();
		return IteratorProxy(mMergeArray.GetBegin(), pvGetCrew().GetVersion());
	}

	Iterator GetEnd() const noexcept
	{
		return Iterator();
	}

	MOMO_FRIEND_SWAP(MergeSet)
	MOMO_FRIENDS_SIZE_BEGIN_END(MergeSet)

	const MergeTraits& GetMergeTraits() const noexcept
	{
		return pvGetCrew().GetContainerTraits();
	}

	const MemManager& GetMemManager() const noexcept
	{
		return pvGetCrew().GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return pvGetCrew().GetMemManager();
	}

	size_t GetCount() const noexcept
	{
		return mMergeArray.GetCount();
	}

	bool IsEmpty() const noexcept
	{
		return mMergeArray.IsEmpty();
	}

	void Clear() noexcept
	{
		pvFilterClear();
		mMergeArray.Clear(true);
		pvGetCrew().IncVersion();
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
		return pvInsert<extraCheck>(key,
			FastMovableFunctor<ItemCreator>(std::forward<ItemCreator>(itemCreator)));
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
		return pvAdd<extraCheck>(pos,
			FastMovableFunctor<ItemCreator>(std::forward<ItemCreator>(itemCreator)));
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
	const Crew& pvGetCrew() const noexcept
	{
		return mMergeArray.GetMemManager().GetMergeSetCrew();
	}

	Crew& pvGetCrew() noexcept
	{
		return mMergeArray.GetMemManager().GetMergeSetCrew();
	}

	Position pvMakePosition(const Item& item) const noexcept
	{
		return PositionProxy(item, pvGetCrew().GetVersion());
	}

	bool pvExtraCheck(ConstPosition pos) const noexcept
	{
		try
		{
			return pos == pvFind(ItemTraits::GetKey(*pos));
		}
		catch (...)
		{
			//?
			return false;
		}
	}

	Position pvFind(const Key& key) const
	{
		const MergeTraits& mergeTraits = GetMergeTraits();
		size_t hashCode = 0;
		if constexpr (MergeTraits::func == MergeTraitsFunc::hash || !BloomFilter::isAlwaysEmpty)
			hashCode = mergeTraits.GetHashCode(key);
		auto itemPred = [&mergeTraits, &key] (const Item& item)
			{ return mergeTraits.IsEqual(ItemTraits::GetKey(item), key); };
		switch (size_t segCount = mMergeArray.GetSegmentCount(); segCount)
		{
		default:
			{
				size_t segIndex = segCount - 1;
				const Item* segItems = mMergeArray.GetSegmentItems(segIndex);
				while (true)
				{
					size_t segItemCount = initialItemCount << (segIndex - 1);
					if constexpr (MergeTraits::func == MergeTraitsFunc::hash)
					{
						auto hashFunc = [&mergeTraits] (const Item& item)
							{ return mergeTraits.GetHashCode(ItemTraits::GetKey(item)); };
						auto equalFunc = [&mergeTraits] (const Item& item1, const auto& item2)
						{
							const Key& key1 = ItemTraits::GetKey(item1);
							if constexpr (std::is_same_v<decltype(item2), const Key&>)
								return mergeTraits.IsEqual(key1, item2);
							else
								return mergeTraits.IsEqual(key1, ItemTraits::GetKey(item2));
						};
						auto findRes = HashSorter::Find(segItems, segItemCount, key,
							hashCode, hashFunc, equalFunc);
						if (findRes.found)
							return pvMakePosition(*findRes.iterator);
					}
					else
					{
						auto lessFunc = [&mergeTraits] (const Item& item1, const Key& key2)
							{ return mergeTraits.IsLess(ItemTraits::GetKey(item1), key2); };
						const Item* itemPtr = std::lower_bound(segItems,
							segItems + segItemCount - 1, key, FastCopyableFunctor(lessFunc));
						if (itemPred(*itemPtr))
							return pvMakePosition(*itemPtr);
					}
					if (segIndex == segCount - 1 && !pvFilterTest(hashCode))
						return Position();
					while (true)
					{
						--segIndex;
						segItems = mMergeArray.GetSegmentItems(segIndex);
						if (segItems != nullptr)
							break;
					}
					if (segIndex < 2)
						break;
				}
			}
			[[fallthrough]];
		case 2:
			if (const Item* segItems = mMergeArray.GetSegmentItems(1); segItems != nullptr)
			{
				const Item* itemPtr = std::find_if(segItems, segItems + initialItemCount,
					FastCopyableFunctor(itemPred));
				if (itemPtr != segItems + initialItemCount)
					return pvMakePosition(*itemPtr);
			}
			[[fallthrough]];
		case 1:
			{
				const Item* segItems = mMergeArray.GetSegmentItems(0);
				size_t segItemCount = ((GetCount() - 1) & (initialItemCount - 1)) + 1;
				const Item* itemPtr = std::find_if(segItems, segItems + segItemCount,
					FastCopyableFunctor(itemPred));
				if (itemPtr != segItems + segItemCount)
					return pvMakePosition(*itemPtr);
			}
			[[fallthrough]];
		case 0:
			return Position();
		}
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
		mMergeArray.AddBackCrt(std::move(itemCreator));
		const Item& item = mMergeArray.GetBackItem();
		pvFilterSet(ItemTraits::GetKey(item));
		pvGetCrew().IncVersion();
		Position resPos = pvMakePosition(item);
		MOMO_EXTRA_CHECK(!extraCheck || pvExtraCheck(resPos));
		return resPos;
	}

	size_t pvGetFilterLogMaxCount() const noexcept
	{
		size_t count = GetCount();
		MOMO_ASSERT(count > 1);
		return static_cast<size_t>(std::bit_width(count - 1) - 1);
	}

	void pvFilterClear() noexcept
	{
		if constexpr (!BloomFilter::isAlwaysEmpty)
		{
			if (!mBloomFilter.IsEmpty())
				mBloomFilter.Clear(GetMemManager(), pvGetFilterLogMaxCount());
		}
	}

	bool pvFilterTest(size_t hashCode) const noexcept
	{
		if constexpr (BloomFilter::isAlwaysEmpty)
			return true;
		else
			return mBloomFilter.IsEmpty() || mBloomFilter.Test(hashCode, pvGetFilterLogMaxCount());
	}

	void pvFilterSet(const Key& key) noexcept
	{
		if constexpr (!BloomFilter::isAlwaysEmpty)
		{
			size_t count = GetCount() - 1;
			if (count < 2 * initialItemCount)
				return;
			size_t logCount = static_cast<size_t>(std::bit_width(count) - 1);
			MemManager& memManager = GetMemManager();
			if (count == size_t{1} << logCount)
			{
				if (!mBloomFilter.IsEmpty())
					mBloomFilter.Clear(memManager, logCount - 1);
				try
				{
					mBloomFilter.Init(memManager, logCount);
				}
				catch (...)
				{
				}
			}
			if (mBloomFilter.IsEmpty())
				return;
			try
			{
				size_t hashCode = GetMergeTraits().GetHashCode(key);
				mBloomFilter.Set(hashCode, logCount);
			}
			catch (...)
			{
				mBloomFilter.Clear(memManager, logCount);
			}
		}
	}

private:
	MergeArray mMergeArray;
	[[no_unique_address]] BloomFilter mBloomFilter;
};

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

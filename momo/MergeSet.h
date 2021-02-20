/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/MergeSet.h

  namespace momo:
    class MergeTraits
    class MergeSetItemTraits
    class MergeSetSettings
    class MergeSet

\**********************************************************/

#pragma once

#include "MergeArray.h"
#include "SetUtility.h"

namespace momo
{

namespace internal
{
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

		~MergeSetNestedArrayMemManager() = default;

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

	template<typename TItemTraits, typename TMemManager>
	class MergeSetNestedArrayItemTraits
	{
	protected:
		typedef TItemTraits ItemTraits;

	public:
		typedef TMemManager MemManager;

		typedef typename ItemTraits::Item Item;

		static const size_t alignment = ItemTraits::alignment;

	public:
		template<typename Iterator>
		static void Destroy(MemManager& memManager, Iterator begin, size_t count) noexcept
		{
			ItemTraits::Destroy(memManager.GetBaseMemManager(), begin, count);
		}

		template<typename SrcIterator, typename ItemCreator>
		static void RelocateCreate(MemManager& memManager, SrcIterator srcBegin, Item* dstBegin,
			size_t count, ItemCreator&& itemCreator, Item* newItem)
		{
			MOMO_ASSERT(std::has_single_bit(count));
			pvCopy(memManager, *pvGetItemPtr(srcBegin, count - 1), dstBegin + count - 1);
			for (size_t index = 1; index < count; index *= 2)
			{
				Item* srcItems1 = pvGetItemPtr(srcBegin, count - 2 * index);
				pvMerge(memManager, srcItems1, dstBegin + count - 2 * index, index);
			}
			try
			{
				std::forward<ItemCreator>(itemCreator)(newItem);
			}
			catch (...)
			{
				Destroy(memManager, dstBegin, count);
				throw;
			}
			Destroy(memManager, srcBegin, count);
		}

	private:
		template<typename Iterator>
		static Item* pvGetItemPtr(Iterator begin, size_t index) noexcept
		{
			return std::addressof(begin[static_cast<ptrdiff_t>(index)]);
		}

		static void pvCopy(MemManager& memManager, const Item& srcItem, Item* dstItem)
		{
			ItemTraits::template Creator<const Item&>(memManager.GetBaseMemManager(), srcItem)(dstItem);
		}

		static void pvMerge(MemManager& memManager, Item* srcItems1, Item* dstItems, size_t count)
		{
			const auto& mergeTraits = memManager.GetMergeSetCrew().GetContainerTraits();
			Item* srcItems2 = dstItems + count;
			size_t srcIndex1 = 0;
			size_t srcIndex2 = 0;
			size_t dstIndex = 0;
			try
			{
				while (srcIndex1 < count)
				{
					if (srcIndex2 < count && mergeTraits.IsLess(
						ItemTraits::GetKey(srcItems2[srcIndex2]),
						ItemTraits::GetKey(srcItems1[srcIndex1])))
					{
						ItemTraits::Relocate(memManager.GetBaseMemManager(),
							srcItems2[srcIndex2], dstItems + dstIndex);
						++srcIndex2;
					}
					else
					{
						pvCopy(memManager, srcItems1[srcIndex1], dstItems + dstIndex);
						++srcIndex1;
					}
					++dstIndex;
				}
			}
			catch (...)
			{
				Destroy(memManager, dstItems, dstIndex);
				Destroy(memManager, srcItems2 + srcIndex2, count - srcIndex2);
				throw;
			}
		}
	};

	template<typename TMergeTraits>
	class MergeSetNestedArraySettings
		: public MergeArraySettings<0 /*TMergeTraits::logInitialItemCount*/>
	{
	protected:
		typedef TMergeTraits MergeTraits;

	public:
		static const CheckMode checkMode = CheckMode::assertion;
	};
}

template<typename TKey,
	size_t tLogInitialItemCount = 5>
class MergeTraits
{
public:
	typedef TKey Key;

	static const size_t logInitialItemCount = tLogInitialItemCount;

public:
	explicit MergeTraits() noexcept
	{
	}

	bool IsLess(const Key& key1, const Key& key2) const
	{
		return std::less<>()(key1, key2);
	}

	bool IsEqual(const Key& key1, const Key& key2) const
	{
		return key1 == key2;
	}
};

template<typename TKey, conceptMemManager TMemManager>
class MergeSetItemTraits
{
public:
	typedef TKey Key;
	typedef TMemManager MemManager;
	typedef Key Item;

private:
	typedef internal::ObjectManager<Item, MemManager> ItemManager;

public:
	static const size_t alignment = ItemManager::alignment;

	template<typename... ItemArgs>
	using Creator = typename ItemManager::template Creator<ItemArgs...>;

public:
	static const Key& GetKey(const Item& item) noexcept
	{
		return item;
	}

	template<typename Iterator>
	static void Destroy(MemManager& memManager, Iterator begin, size_t count) noexcept
	{
		ItemManager::Destroy(memManager, begin, count);
	}

	static void Relocate(MemManager& memManager, Item& srcItem, Item* dstItem)
	{
		ItemManager::Relocate(memManager, srcItem, dstItem);
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

template<typename TKey,
	typename TMergeTraits = MergeTraits<TKey>,
	conceptMemManager TMemManager = MemManagerDefault,
	typename TItemTraits = MergeSetItemTraits<TKey, TMemManager>,
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

	static_assert(internal::ObjectAlignmenter<Item>::Check(ItemTraits::alignment));

private:
	typedef internal::SetCrew<MergeTraits, MemManager, Settings::checkVersion> Crew;

	typedef internal::MergeSetNestedArrayMemManager<Crew> MergeArrayMemManager;
	typedef internal::MergeSetNestedArrayItemTraits<ItemTraits,
		MergeArrayMemManager> MergeArrayItemTraits;

	typedef momo::MergeArray<Item, MergeArrayMemManager, MergeArrayItemTraits,
		internal::MergeSetNestedArraySettings<MergeTraits>> MergeArray;

public:
	typedef typename MergeArray::ConstIterator ConstIterator;
	typedef ConstIterator Iterator;

	typedef const Item* ConstPosition;
	typedef ConstPosition Position;

	typedef internal::InsertResult<ConstPosition> InsertResult;

private:
	template<typename... ItemArgs>
	using Creator = typename ItemTraits::template Creator<ItemArgs...>;

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
		Insert(items);
	}

	MergeSet(MergeSet&& mergeSet) noexcept
		: mMergeArray(std::move(mergeSet.mMergeArray))
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
	}

	~MergeSet() noexcept
	{
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
	}

	ConstIterator GetBegin() const noexcept
	{
		return mMergeArray.GetBegin();
	}

	ConstIterator GetEnd() const noexcept
	{
		return mMergeArray.GetEnd();
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

	void Clear(bool shrink = true) noexcept
	{
		mMergeArray.Clear(shrink);
		pvGetCrew().IncVersion();
	}

	ConstPosition Find(const Key& key) const
	{
		return pvFind(key);
	}

	bool ContainsKey(const Key& key) const
	{
		return pvFind(key) != ConstPosition();
	}

	template<typename ItemCreator>
	requires std::invocable<ItemCreator&&, Item*>
	InsertResult InsertCrt(const Key& key, ItemCreator&& itemCreator)
	{
		return pvInsert<true>(key, std::forward<ItemCreator>(itemCreator));
	}

	template<typename... ItemArgs>
	InsertResult InsertVar(const Key& key, ItemArgs&&... itemArgs)
	{
		return InsertCrt(key,
			Creator<ItemArgs...>(GetMemManager(), std::forward<ItemArgs>(itemArgs)...));
	}

	InsertResult Insert(Item&& item)
	{
		const Key& key = ItemTraits::GetKey(static_cast<const Item&>(item));
		return pvInsert<false>(key, Creator<Item&&>(GetMemManager(), std::move(item)));
	}

	InsertResult Insert(const Item& item)
	{
		return pvInsert<false>(ItemTraits::GetKey(item),
			Creator<const Item&>(GetMemManager(), item));
	}

	template<internal::conceptInputIterator ArgIterator>
	size_t Insert(ArgIterator begin, ArgIterator end)
	{
		MOMO_CHECK_ITERATOR_REFERENCE(ArgIterator, Item);
		size_t count = 0;
		for (ArgIterator iter = begin; iter != end; ++iter)
			count += Insert(*iter).inserted ? size_t{1} : size_t{0};
		return count;
	}

	size_t Insert(std::initializer_list<Item> items)
	{
		return Insert(items.begin(), items.end());
	}

	template<typename ItemCreator,
		bool extraCheck = true>
	requires std::invocable<ItemCreator&&, Item*>
	ConstPosition AddCrt(ItemCreator&& itemCreator)
	{
		return pvAdd<extraCheck>(std::forward<ItemCreator>(itemCreator));
	}

	template<typename... ItemArgs>
	ConstPosition AddVar(ItemArgs&&... itemArgs)
	{
		return AddCrt(Creator<ItemArgs...>(GetMemManager(), std::forward<ItemArgs>(itemArgs)...));
	}

	ConstPosition Add(Item&& item)
	{
		return AddVar(std::move(item));
	}

	ConstPosition Add(const Item& item)
	{
		return AddVar(item);
	}

	//template<typename KeyArg,
	//	bool extraCheck = true>
	//void ResetKey(ConstPosition pos, KeyArg&& keyArg)

private:
	const Crew& pvGetCrew() const noexcept
	{
		return mMergeArray.GetMemManager().GetMergeSetCrew();
	}

	Crew& pvGetCrew() noexcept
	{
		return mMergeArray.GetMemManager().GetMergeSetCrew();
	}

	ConstPosition pvFind(const Key& key) const
	{
		const MergeTraits& mergeTraits = GetMergeTraits();
		auto comp = [&mergeTraits] (const Item& item1, const Key& key2)
			{ return mergeTraits.IsLess(ItemTraits::GetKey(item1), key2); };
		size_t index = 0;
		size_t capacity = mMergeArray.GetCapacity();
		while (capacity > 1)
		{
			size_t segItemCount = std::bit_floor(capacity - 1);
			const Item* segment = std::addressof(mMergeArray[index]);
			const Item* itemPtr = std::lower_bound(segment, segment + segItemCount - 1, key, comp);
			if (mergeTraits.IsEqual(ItemTraits::GetKey(*itemPtr), key))
				return ConstPosition(itemPtr);
			index += segItemCount;
			capacity -= segItemCount;
		}
		if (index < mMergeArray.GetCount())
		{
			const Item* itemPtr = std::addressof(mMergeArray[index]);
			if (mergeTraits.IsEqual(ItemTraits::GetKey(*itemPtr), key))
				return ConstPosition(itemPtr);
		}
		return ConstPosition();
	}

	template<bool extraCheck, typename ItemCreator>
	InsertResult pvInsert(const Key& key, ItemCreator&& itemCreator)
	{
		ConstPosition pos = pvFind(key);
		if (!!pos)
			return { pos, false };
		pos = pvAdd<extraCheck>(std::forward<ItemCreator>(itemCreator));
		return { pos, true };
	}

	template<bool extraCheck, typename ItemCreator>
	ConstPosition pvAdd(ItemCreator&& itemCreator)
	{
		mMergeArray.AddBackCrt(std::forward<ItemCreator>(itemCreator));
		pvGetCrew().IncVersion();
		ConstPosition resPos = std::addressof(mMergeArray.GetBackItem());
		MOMO_EXTRA_CHECK(!extraCheck || resPos == pvFind(ItemTraits::GetKey(*resPos)));
		return resPos;
	}

private:
	MergeArray mMergeArray;
};

} // namespace momo

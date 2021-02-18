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
	template<typename TItemTraits>
	class MergeSetNestedArrayItemTraits
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef typename ItemTraits::MemManager MergeSetMemManager;

	public:
		typedef typename ItemTraits::Item Item;

		typedef MemManagerPtr<MergeSetMemManager> MemManager;

		static const size_t alignment = ItemTraits::alignment;

	public:
		template<typename Iterator>
		static void Destroy(MemManager& memManager, Iterator begin, size_t count) noexcept
		{
			ItemTraits::Destroy(memManager.GetBaseMemManager(), begin, count);
		}

		template<typename SrcIterator, typename DstIterator, typename ItemCreator>
		static void RelocateCreate(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
			size_t count, ItemCreator&& itemCreator, Item* newItem)
		{
			ItemTraits::RelocateCreate(memManager.GetBaseMemManager(), srcBegin, dstBegin, count,
				std::forward<ItemCreator>(itemCreator), newItem);
		}
	};

	template<typename TMergeTraits>
	class MergeSetNestedArraySettings
		: public MergeArraySettings<TMergeTraits::logInitialItemCount>
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

	template<typename SrcIterator, typename DstIterator, typename ItemCreator>
	static void RelocateCreate(MemManager& memManager, SrcIterator srcBegin, DstIterator dstBegin,
		size_t count, ItemCreator&& itemCreator, Item* newItem)
	{
		ItemManager::RelocateCreate(memManager, srcBegin, dstBegin, count,
			std::forward<ItemCreator>(itemCreator), newItem);
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
	typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

	typedef internal::SetCrew<MergeTraits, MemManager, Settings::checkVersion> Crew;

	typedef internal::MergeSetNestedArrayItemTraits<ItemTraits> MergeArrayItemTraits;
	typedef typename MergeArrayItemTraits::MemManager MergeArrayMemManager;

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
		: mCrew(mergeTraits, std::move(memManager)),
		mMergeArray(MergeArrayMemManager(GetMemManager()))
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
		: mCrew(std::move(mergeSet.mCrew)),
		mMergeArray(std::move(mergeSet.mMergeArray))
	{
	}

	MergeSet(const MergeSet& mergeSet)
		: MergeSet(mergeSet, MemManager(mergeSet.GetMemManager()))
	{
	}

	explicit MergeSet(const MergeSet& mergeSet, MemManager memManager)
		: mCrew(mergeSet.GetMergeTraits(), std::move(memManager)),
		mMergeArray(MergeArray::CreateCap(mergeSet.GetCount(), MergeArrayMemManager(GetMemManager())))
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
		mCrew.Swap(mergeSet.mCrew);
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
		return mMergeArray.GetCount();
	}

	bool IsEmpty() const noexcept
	{
		return mMergeArray.IsEmpty();
	}

	void Clear(bool shrink = true) noexcept
	{
		mMergeArray.Clear(shrink);
		mCrew.IncVersion();
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
	ConstPosition pvFind(const Key& key) const
	{
		const MergeTraits& mergeTraits = GetMergeTraits();
		for (const Item& item : mMergeArray)
		{
			if (mergeTraits.IsEqual(key, ItemTraits::GetKey(item)))
				return ConstPosition(std::addressof(item));
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
		mCrew.IncVersion();
		ConstPosition resPos = std::addressof(mMergeArray.GetBackItem());
		MOMO_EXTRA_CHECK(!extraCheck || resPos == pvFind(ItemTraits::GetKey(*resPos)));
		return resPos;
	}

private:
	Crew mCrew;
	MergeArray mMergeArray;
};

} // namespace momo

/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/Array.h

  namespace momo:
    class ArrayItemTraits
    enum class ArrayGrowCause
    class ArraySettings
    class Array
    class ArrayIntCap

\**********************************************************/

#pragma once

#include "ArrayUtility.h"
#include "KeyUtility.h"

namespace momo
{

namespace internal
{
	template<typename ItemTraits, size_t count>
	class ArrayBuffer : public ObjectBuffer<typename ItemTraits::Item, ItemTraits::alignment, count>
	{
	};

	template<typename ItemTraits>
	class ArrayBuffer<ItemTraits, 0>
	{
	};

	template<typename Array,
		bool usePtrIterator = Array::Settings::usePtrIterator>
	class ArrayIteratorSelector;

	template<typename Array>
	class ArrayIteratorSelector<Array, true>
	{
	public:
		typedef const typename Array::Item* ConstIterator;
		typedef typename Array::Item* Iterator;

	public:
		static ConstIterator MakeIterator(const Array& array, size_t index) noexcept
		{
			return array.GetItems() + index;
		}

		static Iterator MakeIterator(Array& array, size_t index) noexcept
		{
			return array.GetItems() + index;
		}
	};

	template<typename Array>
	class ArrayIteratorSelector<Array, false>
	{
	public:
		typedef ArrayIndexIterator<Array, typename Array::Item> Iterator;
		typedef typename Iterator::ConstIterator ConstIterator;

	private:
		struct ConstIteratorProxy : public ConstIterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
		};

		struct IteratorProxy : public Iterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
		};

	public:
		static ConstIterator MakeIterator(const Array& array, size_t index) noexcept
		{
			return ConstIteratorProxy(&array, index);
		}

		static Iterator MakeIterator(Array& array, size_t index) noexcept
		{
			return IteratorProxy(&array, index);
		}
	};
}

template<conceptObject TItem, conceptMemManager TMemManager>
class ArrayItemTraits
{
public:
	typedef TItem Item;
	typedef TMemManager MemManager;

private:
	typedef internal::ObjectManager<Item, MemManager> ItemManager;

public:
	static const size_t alignment = ItemManager::alignment;

	static const bool isNothrowMoveConstructible = ItemManager::isNothrowMoveConstructible;
	static const bool isTriviallyRelocatable = ItemManager::isTriviallyRelocatable;
	static const bool isNothrowRelocatable = ItemManager::isNothrowRelocatable;

	template<typename... ItemArgs>
	using Creator = typename ItemManager::template Creator<ItemArgs...>;

public:
	static void Destroy(MemManager& memManager, Item* items, size_t count) noexcept
	{
		ItemManager::Destroy(memManager, items, count);
	}

	static void Relocate(MemManager& memManager, Item* srcItems, Item* dstItems, size_t count)
		noexcept(isNothrowRelocatable)
	{
		ItemManager::Relocate(memManager, srcItems, dstItems, count);
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	static void RelocateCreate(MemManager& memManager, Item* srcItems, Item* dstItems,
		size_t count, FastMovableFunctor<ItemCreator> itemCreator, Item* newItem)
	{
		ItemManager::RelocateCreate(memManager, srcItems, dstItems, count,
			std::move(itemCreator), newItem);
	}

	template<typename ItemArg>
	static void Assign(MemManager& /*memManager*/, ItemArg&& itemArg, Item& item)
	{
		item = std::forward<ItemArg>(itemArg);
	}
};

enum class ArrayGrowCause
{
	add = 0,
	reserve = 1,
};

template<size_t tInternalCapacity = 0,
	bool tGrowOnReserve = true,
	bool tUsePtrIterator = (tInternalCapacity == 0)>
class ArraySettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;

	static const size_t internalCapacity = tInternalCapacity;
	static const bool growOnReserve = tGrowOnReserve;
	static const bool usePtrIterator = tUsePtrIterator;

public:
	static size_t GrowCapacity(size_t capacity, size_t minNewCapacity,
		ArrayGrowCause growCause, bool linear) noexcept
	{
		MOMO_ASSERT(capacity < minNewCapacity);
		if (growCause == ArrayGrowCause::reserve && !growOnReserve)
			return minNewCapacity;
		size_t newCapacity;
		if (capacity <= 2)
			newCapacity = 4;
		else if (capacity <= 64)
			newCapacity = capacity * 2;
		else if (linear || capacity < 150)
			newCapacity = capacity + 64;
		else
			newCapacity = capacity + (capacity / 50) * 23;	// k^4 < 1 + k + k^2
		if (newCapacity < minNewCapacity)
			newCapacity = minNewCapacity;
		return newCapacity;
	}
};

/*!
	All `Array` functions and constructors have strong exception safety,
	but not the following cases:
	 - Functions `Insert`, `InsertVar`, `InsertCrt`, `Remove` have
	basic exception safety.
*/

template<conceptObject TItem,
	conceptMemManager TMemManager = MemManagerDefault,
	typename TItemTraits = ArrayItemTraits<TItem, TMemManager>,
	typename TSettings = ArraySettings<>>
class Array
{
public:
	typedef TItem Item;
	typedef TMemManager MemManager;
	typedef TItemTraits ItemTraits;
	typedef TSettings Settings;

	static const size_t internalCapacity = Settings::internalCapacity;

private:
	class Data : private MemManager
	{
	private:
		typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

	public:
		explicit Data(MemManager&& memManager) noexcept
			: MemManager(std::move(memManager))
		{
			pvInit();
		}

		explicit Data(size_t capacity, MemManager&& memManager)
			: MemManager(std::move(memManager))
		{
			if (capacity > internalCapacity)
			{
				mItems = pvAllocate(capacity);
				mCount = 0;
				mCapacity = capacity;
			}
			else
			{
				pvInit();
			}
		}

		Data(Data&& data) noexcept
			: MemManager(std::move(data.GetMemManager()))
		{
			pvInit(std::move(data));
		}

		Data(const Data&) = delete;

		~Data() noexcept
		{
			pvDestroy();
		}

		Data& operator=(Data&& data) noexcept
		{
			if (this != &data)
			{
				pvDestroy();
				MemManagerProxy::Assign(std::move(data.GetMemManager()), GetMemManager());
				pvInit(std::move(data));
			}
			return *this;
		}

		Data& operator=(const Data&) = delete;

		const Item* GetItems() const noexcept
		{
			return mItems;
		}

		Item* GetItems() noexcept
		{
			return mItems;
		}

		const MemManager& GetMemManager() const noexcept
		{
			return *this;
		}

		MemManager& GetMemManager() noexcept
		{
			return *this;
		}

		size_t GetCapacity() const noexcept
		{
			return pvIsInternal() ? internalCapacity : mCapacity;
		}

		size_t GetCount() const noexcept
		{
			return mCount;
		}

		void SetCount(size_t count) noexcept
		{
			MOMO_ASSERT(count <= GetCapacity());
			mCount = count;
		}

		void Clear() noexcept
		{
			pvDestroy();
			pvInit();
		}

		bool Reallocate(size_t capacityLin, size_t capacityExp)
		{
			if (GetCapacity() == internalCapacity)
				return false;
			if (capacityLin <= internalCapacity || capacityExp <= internalCapacity)
				return false;
			if constexpr (MemManagerProxy::canReallocateInplace)
			{
				if (!pvCanReallocate() || capacityLin < capacityExp)
				{
					pvCheckCapacity(capacityLin);
					if (MemManagerProxy::ReallocateInplace(GetMemManager(),
						mItems, mCapacity * sizeof(Item), capacityLin * sizeof(Item)))
					{
						mCapacity = capacityLin;
						return true;
					}
				}
			}
			if constexpr (pvCanReallocate())
			{
				pvCheckCapacity(capacityExp);
				mItems = MemManagerProxy::Reallocate(GetMemManager(),
					mItems, mCapacity * sizeof(Item), capacityExp * sizeof(Item));
				mCapacity = capacityExp;
				return true;
			}
			else
			{
				return false;
			}
		}

		template<bool grow, internal::conceptMoveFunctor<void, Item*> ItemsCreator>
		void Reset(size_t capacity, size_t count, FastMovableFunctor<ItemsCreator> itemsCreator)
		{
			MOMO_ASSERT(count <= capacity);
			if (grow || capacity > internalCapacity)
			{
				Item* items = pvAllocate(capacity);
				try
				{
					std::move(itemsCreator)(items);
				}
				catch (...)
				{
					pvDeallocate(items, capacity);
					throw;
				}
				if (!pvIsInternal() && mCapacity > 0)
					pvDeallocate(mItems, mCapacity);
				mItems = items;
				mCount = count;
				mCapacity = capacity;
			}
			else if constexpr (internalCapacity == 0)
			{
				MOMO_ASSERT(count == 0);
				MOMO_ASSERT(mCapacity > 0);
				pvDeallocate(mItems, mCapacity);
				pvInit();
			}
			else
			{
				MOMO_ASSERT(!pvIsInternal());
				size_t initCapacity = mCapacity;
				Item* items = pvActivateInternalItems();
				try
				{
					std::move(itemsCreator)(items);
				}
				catch (...)
				{
					mCapacity = initCapacity;	// deactivate mInternalItems
					throw;
				}
				pvDeallocate(mItems, initCapacity);
				mItems = items;
				mCount = count;
			}
		}

	private:
		static void pvCheckCapacity(size_t capacity)
		{
			if (capacity > internal::UIntConst::maxSize / sizeof(Item))
				throw std::bad_array_new_length();
		}

		Item* pvAllocate(size_t capacity)
		{
			pvCheckCapacity(capacity);
			static_assert(internal::ObjectAlignmenter<Item>::Check(ItemTraits::alignment));
			return MemManagerProxy::template Allocate<Item>(GetMemManager(),
				capacity * sizeof(Item));
		}

		void pvDeallocate(Item* items, size_t capacity) noexcept
		{
			MemManagerProxy::Deallocate(GetMemManager(), items, capacity * sizeof(Item));
		}

		void pvInit() noexcept
		{
			if constexpr (internalCapacity == 0)
			{
				mItems = nullptr;
				mCapacity = 0;
			}
			else
			{
				mItems = pvActivateInternalItems();
			}
			mCount = 0;
		}

		void pvInit(Data&& data) noexcept
		{
			bool inited = false;
			if constexpr (internalCapacity > 0)
			{
				static_assert(ItemTraits::isNothrowRelocatable);
				if (data.pvIsInternal())
				{
					mItems = pvActivateInternalItems();
					ItemTraits::Relocate(GetMemManager(), data.mItems, mItems, data.mCount);
					inited = true;
				}
			}
			if (!inited)
			{
				mItems = data.mItems;
				mCapacity = data.mCapacity;
			}
			mCount = data.mCount;
			data.pvInit();
		}

		Item* pvActivateInternalItems() noexcept
		{
			return &*std::construct_at(std::addressof(mInternalItems));
		}

		void pvDestroy() noexcept
		{
			ItemTraits::Destroy(GetMemManager(), mItems, mCount);
			if (!pvIsInternal() && mCapacity > 0)
				pvDeallocate(mItems, mCapacity);
		}

		bool pvIsInternal() const noexcept
		{
			return (internalCapacity == 0) ? false
				: static_cast<void*>(mItems) == std::addressof(mInternalItems);
		}

		static constexpr bool pvCanReallocate() noexcept
		{
			if constexpr (MemManagerProxy::canReallocate)
				return ItemTraits::isTriviallyRelocatable;
			else
				return false;
		}

	private:
		Item* mItems;
		size_t mCount;
		union
		{
			size_t mCapacity;
			internal::ArrayBuffer<ItemTraits, internalCapacity> mInternalItems;
		};
	};

	typedef internal::ArrayItemHandler<ItemTraits> ItemHandler;
	typedef internal::ArrayShifter<Array> ArrayShifter;
	typedef typename internal::ArrayIteratorSelector<Array> IteratorSelector;

	typedef internal::UIntMath<> SMath;

public:
	typedef typename IteratorSelector::ConstIterator ConstIterator;
	typedef typename IteratorSelector::Iterator Iterator;

public:
	Array() noexcept(noexcept(MemManager()))
		: Array(MemManager())
	{
	}

	explicit Array(MemManager memManager) noexcept
		: mData(std::move(memManager))
	{
	}

	explicit Array(size_t count, MemManager memManager = MemManager())
		: mData(count, std::move(memManager))
	{
		for (size_t i = 0; i < count; ++i)
			AddBackNogrowVar();
	}

	explicit Array(size_t count, const Item& item, MemManager memManager = MemManager())
		: mData(count, std::move(memManager))
	{
		for (size_t i = 0; i < count; ++i)
			AddBackNogrow(item);
	}

	template<std::input_iterator ArgIterator, internal::conceptSentinel<ArgIterator> ArgSentinel>
	explicit Array(ArgIterator begin, ArgSentinel end, MemManager memManager = MemManager())
		: Array(std::move(begin), std::move(end), std::move(memManager), nullptr)
	{
	}

	Array(std::initializer_list<Item> items)
		: Array(items.begin(), items.end())
	{
	}

	explicit Array(std::initializer_list<Item> items, MemManager memManager)
		: Array(items.begin(), items.end(), std::move(memManager))
	{
	}

	Array(Array&& array) noexcept
		: mData(std::move(array.mData))
	{
	}

	Array(const Array& array)
		: Array(array, true)
	{
	}

	explicit Array(const Array& array, bool shrink)
		: mData(shrink ? array.GetCount() : array.GetCapacity(), MemManager(array.GetMemManager()))
	{
		for (const Item& item : array)
			AddBackNogrow(item);
	}

	explicit Array(const Array& array, MemManager memManager)
		: Array(array.GetBegin(), array.GetEnd(), std::move(memManager))
	{
	}

	static Array CreateCap(size_t capacity, MemManager memManager = MemManager())
	{
		return Array(Data(capacity, std::move(memManager)));
	}

	template<internal::conceptObjectMultiCreator<Item> ItemMultiCreator>
	static Array CreateCrt(size_t count, ItemMultiCreator itemMultiCreator,
		MemManager memManager = MemManager())
	{
		FastCopyableFunctor<ItemMultiCreator> fastItemMultiCreator(itemMultiCreator);
		Array array = CreateCap(count, std::move(memManager));
		for (size_t i = 0; i < count; ++i)
			array.pvAddBackNogrow(FastMovableFunctor(FastCopyableFunctor(fastItemMultiCreator)));
		return array;
	}

	~Array() noexcept = default;

	Array& operator=(Array&& array) noexcept
	{
		mData = std::move(array.mData);
		return *this;
	}

	Array& operator=(const Array& array)
	{
		if (this != &array)
			*this = Array(array);
		return *this;
	}

	void Swap(Array& array) noexcept
	{
		if (this != &array)
			std::swap(mData, array.mData);
	}

	ConstIterator GetBegin() const noexcept
	{
		return IteratorSelector::MakeIterator(*this, 0);
	}

	Iterator GetBegin() noexcept
	{
		return IteratorSelector::MakeIterator(*this, 0);
	}

	ConstIterator GetEnd() const noexcept
	{
		return IteratorSelector::MakeIterator(*this, GetCount());
	}

	Iterator GetEnd() noexcept
	{
		return IteratorSelector::MakeIterator(*this, GetCount());
	}

	MOMO_FRIEND_SWAP(Array)
	MOMO_FRIENDS_SIZE_BEGIN_END(Array)

	const Item* GetItems() const noexcept
	{
		return mData.GetItems();
	}

	Item* GetItems() noexcept
	{
		return mData.GetItems();
	}

	const MemManager& GetMemManager() const noexcept
	{
		return mData.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mData.GetMemManager();
	}

	size_t GetCount() const noexcept
	{
		return mData.GetCount();
	}

	template<internal::conceptObjectMultiCreator<Item> ItemMultiCreator>
	void SetCountCrt(size_t count, ItemMultiCreator itemMultiCreator)
	{
		pvSetCount(count, FastCopyableFunctor<ItemMultiCreator>(itemMultiCreator));
	}

	void SetCount(size_t count)
	{
		typedef typename ItemTraits::template Creator<> ItemCreator;
		MemManager& memManager = GetMemManager();
		auto itemMultiCreator = [&memManager] (Item* newItem)
			{ (ItemCreator(memManager))(newItem); };
		pvSetCount(count, FastCopyableFunctor(itemMultiCreator));
	}

	void SetCount(size_t count, const Item& item)
	{
		typedef typename ItemTraits::template Creator<const Item&> ItemCreator;
		MemManager& memManager = GetMemManager();
		auto itemMultiCreator = [&memManager, &item] (Item* newItem)
			{ ItemCreator(memManager, item)(newItem); };
		pvSetCount(count, FastCopyableFunctor(itemMultiCreator));
	}

	bool IsEmpty() const noexcept
	{
		return GetCount() == 0;
	}

	void Clear(bool shrink = false) noexcept
	{
		if (shrink)
			mData.Clear();
		else
			pvRemoveBack(GetCount());
	}

	size_t GetCapacity() const noexcept
	{
		return mData.GetCapacity();
	}

	void Reserve(size_t capacity)
	{
		if (capacity > GetCapacity())
			pvGrow(capacity, ArrayGrowCause::reserve);
	}

	void Shrink()
	{
		Shrink(GetCount());
	}

	void Shrink(size_t capacity)
	{
		size_t initCapacity = GetCapacity();
		if (initCapacity <= capacity || initCapacity == internalCapacity)
			return;
		size_t count = GetCount();
		if (capacity < count)
			capacity = count;
		if (!mData.Reallocate(capacity, capacity))
		{
			auto itemsCreator = [this, count] (Item* newItems)
				{ ItemTraits::Relocate(GetMemManager(), GetItems(), newItems, count); };
			mData.template Reset<false>(capacity, count,
				FastMovableFunctor(std::move(itemsCreator)));
		}
	}

	const Item& operator[](size_t index) const
	{
		MOMO_CHECK(index < GetCount());
		return GetItems()[index];
	}

	Item& operator[](size_t index)
	{
		MOMO_CHECK(index < GetCount());
		return GetItems()[index];
	}

	const Item& GetBackItem(size_t revIndex = 0) const
	{
		return operator[](GetCount() - 1 - revIndex);
	}

	Item& GetBackItem(size_t revIndex = 0)
	{
		return operator[](GetCount() - 1 - revIndex);
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void AddBackNogrowCrt(ItemCreator itemCreator)
	{
		MOMO_CHECK(GetCount() < GetCapacity());
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
		typedef typename ItemTraits::template Creator<Item&&> ItemCreator;
		MemManager& memManager = GetMemManager();
		size_t initCount = GetCount();
		if (initCount < GetCapacity())
		{
			pvAddBackNogrow(FastMovableFunctor(ItemCreator(memManager, std::move(item))));
		}
		else if constexpr (ItemTraits::isNothrowMoveConstructible)
		{
			size_t newCount = initCount + 1;
			size_t itemIndex = pvIndexOf(std::as_const(item));
			pvGrow(newCount, ArrayGrowCause::add);
			Item* items = GetItems();
			ItemCreator(memManager,
				std::move((itemIndex == internal::UIntConst::maxSize) ? item : items[itemIndex]))
					(items + initCount);
			mData.SetCount(newCount);
		}
		else
		{
			pvAddBackGrow(FastMovableFunctor(ItemCreator(memManager, std::move(item))));
		}
	}

	void AddBack(const Item& item)
	{
		typedef typename ItemTraits::template Creator<const Item&> ItemCreator;
		MemManager& memManager = GetMemManager();
		size_t initCount = GetCount();
		if (initCount < GetCapacity())
		{
			pvAddBackNogrow(FastMovableFunctor(ItemCreator(memManager, item)));
		}
		else if constexpr (ItemTraits::isNothrowRelocatable)
		{
			size_t newCount = initCount + 1;
			ItemHandler itemHandler(memManager,
				FastMovableFunctor(ItemCreator(memManager, item)));
			pvGrow(newCount, ArrayGrowCause::add);
			ItemTraits::Relocate(memManager, &itemHandler, GetItems() + initCount, 1);
			itemHandler.Release();
			mData.SetCount(newCount);
		}
		else
		{
			pvAddBackGrow(FastMovableFunctor(ItemCreator(memManager, item)));
		}
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
		size_t initCount = GetCount();
		size_t grow = (initCount + 1 > GetCapacity());
		size_t itemIndex = pvIndexOf(std::as_const(item));
		if (grow || (index <= itemIndex && itemIndex < initCount))
			InsertVar(index, std::move(item));
		else
			ArrayShifter::InsertNogrow(*this, index, std::move(item));
	}

	void Insert(size_t index, const Item& item)
	{
		return Insert(index, size_t{1}, item);	//?
	}

	void Insert(size_t index, size_t count, const Item& item)
	{
		size_t initCount = GetCount();
		size_t newCount = initCount + count;
		size_t grow = (newCount > GetCapacity());
		size_t itemIndex = pvIndexOf(item);
		if (grow || (index <= itemIndex && itemIndex < initCount))
		{
			typedef typename ItemTraits::template Creator<const Item&> ItemCreator;
			MemManager& memManager = GetMemManager();
			ItemHandler itemHandler(memManager, FastMovableFunctor(ItemCreator(memManager, item)));
			if (grow)
				pvGrow(newCount, ArrayGrowCause::add);
			ArrayShifter::InsertNogrow(*this, index, count, *&itemHandler);
		}
		else
		{
			ArrayShifter::InsertNogrow(*this, index, count, item);
		}
	}

	template<std::input_iterator ArgIterator, internal::conceptSentinel<ArgIterator> ArgSentinel>
	void Insert(size_t index, ArgIterator begin, ArgSentinel end)
	{
		MOMO_ASSERT(begin == end || !pvIsInside(*begin));	//?
		if constexpr (internal::conceptForwardIterator<ArgIterator>)
		{
			size_t count = SMath::Dist(begin, end);
			size_t newCount = GetCount() + count;
			if (newCount > GetCapacity())
				pvGrow(newCount, ArrayGrowCause::add);
			ArrayShifter::InsertNogrow(*this, index, begin, count);
		}
		else
		{
			ArrayShifter::Insert(*this, index, std::move(begin), std::move(end));
		}
	}

	void Insert(size_t index, std::initializer_list<Item> items)
	{
		Insert(index, items.begin(), items.end());
	}

	void RemoveBack(size_t count = 1)
	{
		MOMO_CHECK(count <= GetCount());
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
	bool IsEqual(const Array& array, EqualFunc equalFunc = EqualFunc()) const
	{
		return std::equal(GetBegin(), GetEnd(), array.GetBegin(), array.GetEnd(),
			FastCopyableFunctor<EqualFunc>(equalFunc));
	}

private:
	explicit Array(Data&& data) noexcept
		: mData(std::move(data))
	{
	}

	template<internal::conceptForwardIterator ArgIterator,
		internal::conceptSentinel<ArgIterator> ArgSentinel>
	explicit Array(ArgIterator begin, ArgSentinel end, MemManager&& memManager, std::nullptr_t)
		: mData(SMath::Dist(begin, end), std::move(memManager))
	{
		typedef typename ItemTraits::template Creator<std::iter_reference_t<ArgIterator>> IterCreator;
		MemManager& thisMemManager = GetMemManager();
		for (ArgIterator iter = begin; iter != end; ++iter)
			pvAddBackNogrow(FastMovableFunctor(IterCreator(thisMemManager, *iter)));
	}

	template<std::input_iterator ArgIterator, internal::conceptSentinel<ArgIterator> ArgSentinel>
	explicit Array(ArgIterator begin, ArgSentinel end, MemManager&& memManager, std::nullptr_t)
		: mData(std::move(memManager))
	{
		typedef typename ItemTraits::template Creator<std::iter_reference_t<ArgIterator>> IterCreator;
		MemManager& thisMemManager = GetMemManager();
		for (ArgIterator iter = std::move(begin); iter != end; ++iter)
			pvAddBack(FastMovableFunctor(IterCreator(thisMemManager, *iter)));
	}

	size_t pvGrowCapacity(size_t minNewCapacity, ArrayGrowCause growCause, bool linear) const
	{
		size_t newCapacity = Settings::GrowCapacity(GetCapacity(),
			minNewCapacity, growCause, linear);
		MOMO_ASSERT(newCapacity >= minNewCapacity);
		return newCapacity;
	}

	void pvGrow(size_t minNewCapacity, ArrayGrowCause growCause)
	{
		size_t newCapacityLin = pvGrowCapacity(minNewCapacity, growCause, true);
		size_t newCapacityExp = pvGrowCapacity(minNewCapacity, growCause, false);
		if (!mData.Reallocate(newCapacityLin, newCapacityExp))
		{
			size_t count = GetCount();
			auto itemsCreator = [this, count] (Item* newItems)
				{ ItemTraits::Relocate(GetMemManager(), GetItems(), newItems, count); };
			mData.template Reset<true>(newCapacityExp, count,
				FastMovableFunctor(std::move(itemsCreator)));
		}
	}

	template<internal::conceptObjectMultiCreator<Item> ItemMultiCreator>
	void pvSetCount(size_t count, FastCopyableFunctor<ItemMultiCreator> itemMultiCreator)
	{
		size_t newCount = count;
		size_t initCount = GetCount();
		if (newCount <= initCount)
		{
			pvRemoveBack(initCount - newCount);
		}
		else if (newCount <= GetCapacity())
		{
			Item* items = GetItems();
			size_t index = initCount;
			try
			{
				for (; index < newCount; ++index)
					itemMultiCreator(items + index);
			}
			catch (...)
			{
				ItemTraits::Destroy(GetMemManager(), items + initCount, index - initCount);
				throw;
			}
			mData.SetCount(newCount);
		}
		else
		{
			size_t newCapacity = pvGrowCapacity(newCount, ArrayGrowCause::reserve, false);
			auto itemsCreator = [this, initCount, newCount, itemMultiCreator] (Item* newItems)
			{
				size_t index = initCount;
				try
				{
					for (; index < newCount; ++index)
						itemMultiCreator(newItems + index);
					ItemTraits::Relocate(GetMemManager(), GetItems(), newItems, initCount);
				}
				catch (...)
				{
					ItemTraits::Destroy(GetMemManager(), newItems + initCount, index - initCount);
					throw;
				}
			};
			mData.template Reset<true>(newCapacity, newCount,
				FastMovableFunctor(std::move(itemsCreator)));
		}
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void pvAddBack(FastMovableFunctor<ItemCreator> itemCreator)
	{
		if (GetCount() < GetCapacity())
			pvAddBackNogrow(std::move(itemCreator));
		else
			pvAddBackGrow(std::move(itemCreator));
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void pvAddBackNogrow(FastMovableFunctor<ItemCreator> itemCreator)
	{
		size_t count = GetCount();
		std::move(itemCreator)(GetItems() + count);
		mData.SetCount(count + 1);
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void pvAddBackGrow(FastMovableFunctor<ItemCreator> itemCreator)
	{
		size_t initCount = GetCount();
		size_t newCount = initCount + 1;
		size_t newCapacity = pvGrowCapacity(newCount, ArrayGrowCause::add, false);
		auto itemsCreator = [this, initCount, itemCreator = std::move(itemCreator)]
			(Item* newItems) mutable
		{
			ItemTraits::RelocateCreate(GetMemManager(), GetItems(), newItems, initCount,
				std::move(itemCreator), newItems + initCount);
		};
		mData.template Reset<true>(newCapacity, newCount,
			FastMovableFunctor(std::move(itemsCreator)));
	}

	template<internal::conceptObjectCreator<Item> ItemCreator>
	void pvInsert(size_t index, FastMovableFunctor<ItemCreator> itemCreator)
	{
		ItemHandler itemHandler(GetMemManager(), std::move(itemCreator));
		size_t newCount = GetCount() + 1;
		if (newCount > GetCapacity())
			pvGrow(newCount, ArrayGrowCause::add);
		ArrayShifter::InsertNogrow(*this, index, std::move(*&itemHandler));
	}

	void pvRemoveBack(size_t count) noexcept
	{
		size_t initCount = GetCount();
		ItemTraits::Destroy(GetMemManager(), GetItems() + initCount - count, count);
		mData.SetCount(initCount - count);
	}

	size_t pvIndexOf(const Item& item) const noexcept
	{
		const Item* itemPtr = std::addressof(item);
		const Item* items = GetItems();
		std::less<const Item*> less;
		return (!less(itemPtr, items) && less(itemPtr, items + GetCount()))
			? SMath::Dist(items, itemPtr) : internal::UIntConst::maxSize;
	}

	template<typename ItemArg>
	bool pvIsInside(const ItemArg& itemArg) const noexcept
	{
		const void* itemPtr = std::addressof(itemArg);
		const Item* items = GetItems();
		std::less<const void*> less;
		return !less(itemPtr, items) && less(itemPtr, items + GetCount());
	}

private:
	Data mData;
};

template<size_t tInternalCapacity, conceptObject TItem,
	conceptMemManager TMemManager = MemManagerDefault,
	typename TItemTraits = ArrayItemTraits<TItem, TMemManager>>
using ArrayIntCap = Array<TItem, TMemManager, TItemTraits, ArraySettings<tInternalCapacity>>;

namespace internal
{
	template<typename TBaseArraySettings = ArraySettings<>>
	class NestedArraySettings : public TBaseArraySettings
	{
	protected:
		typedef TBaseArraySettings BaseArraySettings;

	public:
		static const CheckMode checkMode = CheckMode::assertion;
	};

	template<size_t tInternalCapacity, conceptObject TItem, conceptMemManager TMemManager>
	using NestedArrayIntCap = Array<TItem, TMemManager, ArrayItemTraits<TItem, TMemManager>,
		NestedArraySettings<ArraySettings<tInternalCapacity>>>;
}

} // namespace momo

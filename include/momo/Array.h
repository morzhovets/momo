/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/Array.h

  namespace momo:
    class ArrayItemTraits
    enum class ArrayGrowCause
    class ArraySettings
    class Array
    class ArrayIntCap

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_ARRAY
#define MOMO_INCLUDE_GUARD_ARRAY

#include "ArrayUtility.h"

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

template<typename TItem, typename TMemManager>
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

	template<typename ItemCreator>
	static void RelocateCreate(MemManager& memManager, Item* srcItems, Item* dstItems,
		size_t count, ItemCreator&& itemCreator, Item* newItem)
	{
		ItemManager::RelocateCreate(memManager, srcItems, dstItems, count,
			std::forward<ItemCreator>(itemCreator), newItem);
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

template<typename TItem,
	typename TMemManager = MemManagerDefault,
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
			pvCheckCapacity(capacity);
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
			static const bool canReallocateInplace = MemManagerProxy::canReallocateInplace;
			static const bool canReallocate = MemManagerProxy::canReallocate
				&& ItemTraits::isTriviallyRelocatable;
			if (!canReallocate || capacityLin < capacityExp)
			{
				if (pvReallocateInplace(capacityLin, internal::BoolConstant<canReallocateInplace>()))
					return true;
			}
			return pvReallocate(capacityExp, internal::BoolConstant<canReallocate>());
		}

		template<typename ItemsCreator>
		void Reset(size_t capacity, size_t count, ItemsCreator&& itemsCreator)
		{
			MOMO_ASSERT(count <= capacity);
			pvCheckCapacity(capacity);
			if (capacity > internalCapacity)
			{
				Item* items = pvAllocate(capacity);
				try
				{
					std::forward<ItemsCreator>(itemsCreator)(items);
				}
				catch (...)
				{
					MemManagerProxy::Deallocate(GetMemManager(), items, capacity * sizeof(Item));
					throw;
				}
				pvDeallocate();
				mItems = items;
				mCount = count;
				mCapacity = capacity;
			}
			else
			{
				pvReset(count, std::forward<ItemsCreator>(itemsCreator));
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
			MOMO_STATIC_ASSERT(internal::ObjectAlignmenter<Item>::Check(ItemTraits::alignment));
			return MemManagerProxy::template Allocate<Item>(GetMemManager(),
				capacity * sizeof(Item));
		}

		template<bool hasInternalCapacity = (internalCapacity > 0)>
		internal::EnableIf<hasInternalCapacity>
		pvInit() noexcept
		{
			mItems = &mInternalItems;
			mCount = 0;
		}

		template<bool hasInternalCapacity = (internalCapacity > 0)>
		internal::EnableIf<!hasInternalCapacity>
		pvInit() noexcept
		{
			mItems = nullptr;
			mCount = 0;
			mCapacity = 0;
		}

		template<bool hasInternalCapacity = (internalCapacity > 0)>
		internal::EnableIf<hasInternalCapacity>
		pvInit(Data&& data) noexcept
		{
			MOMO_STATIC_ASSERT(ItemTraits::isNothrowRelocatable);
			if (data.pvIsInternal())
			{
				mItems = &mInternalItems;
				ItemTraits::Relocate(GetMemManager(), data.mItems, mItems, data.mCount);
			}
			else
			{
				mItems = data.mItems;
				mCapacity = data.mCapacity;
			}
			mCount = data.mCount;
			data.pvInit();
		}

		template<bool hasInternalCapacity = (internalCapacity > 0)>
		internal::EnableIf<!hasInternalCapacity>
		pvInit(Data&& data) noexcept
		{
			mItems = data.mItems;
			mCount = data.mCount;
			mCapacity = data.mCapacity;
			data.pvInit();
		}

		void pvDestroy() noexcept
		{
			ItemTraits::Destroy(GetMemManager(), mItems, mCount);
			pvDeallocate();
		}

		void pvDeallocate() noexcept
		{
			if (GetCapacity() > internalCapacity)
				MemManagerProxy::Deallocate(GetMemManager(), mItems, mCapacity * sizeof(Item));
		}

		template<bool hasInternalCapacity = (internalCapacity > 0)>
		internal::EnableIf<hasInternalCapacity,
		bool> pvIsInternal() const noexcept
		{
			return mItems == &mInternalItems;
		}

		template<bool hasInternalCapacity = (internalCapacity > 0)>
		internal::EnableIf<!hasInternalCapacity,
		bool> pvIsInternal() const noexcept
		{
			return false;
		}

		bool pvReallocateInplace(size_t capacity, std::true_type /*canReallocateInplace*/)
		{
			pvCheckCapacity(capacity);
			if (!MemManagerProxy::ReallocateInplace(GetMemManager(),
				mItems, mCapacity * sizeof(Item), capacity * sizeof(Item)))
			{
				return false;
			}
			mCapacity = capacity;
			return true;
		}

		bool pvReallocateInplace(size_t /*capacity*/,
			std::false_type /*canReallocateInplace*/) noexcept
		{
			return false;
		}

		bool pvReallocate(size_t capacity, std::true_type /*canReallocate*/)
		{
			pvCheckCapacity(capacity);
			mItems = MemManagerProxy::template Reallocate<Item>(GetMemManager(),
				mItems, mCapacity * sizeof(Item), capacity * sizeof(Item));
			mCapacity = capacity;
			return true;
		}

		bool pvReallocate(size_t /*capacity*/, std::false_type /*canReallocate*/) noexcept
		{
			return false;
		}

		template<typename ItemsCreator,
			bool hasInternalCapacity = (internalCapacity > 0)>
		internal::EnableIf<hasInternalCapacity>
		pvReset(size_t count, ItemsCreator&& itemsCreator)
		{
			MOMO_ASSERT(!pvIsInternal());
			size_t initCapacity = mCapacity;
			std::forward<ItemsCreator>(itemsCreator)(&mInternalItems);
			MemManagerProxy::Deallocate(GetMemManager(), mItems, initCapacity * sizeof(Item));
			mItems = &mInternalItems;
			mCount = count;
		}

		template<typename ItemsCreator,
			bool hasInternalCapacity = (internalCapacity > 0)>
		internal::EnableIf<!hasInternalCapacity>
		pvReset(size_t count, ItemsCreator&& /*itemsCreator*/) noexcept
		{
			(void)count;
			MOMO_ASSERT(count == 0);
			pvDeallocate();
			pvInit();
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
	Array() noexcept(std::is_nothrow_default_constructible<MemManager>::value)	// vs2017
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

	template<typename ArgIterator, typename ArgSentinel,
		typename = decltype(*std::declval<ArgIterator>())>
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

	template<typename MultiItemCreator>
	static Array CreateCrt(size_t count, const MultiItemCreator& multiItemCreator,
		MemManager memManager = MemManager())
	{
		Array array = CreateCap(count, std::move(memManager));
		for (size_t i = 0; i < count; ++i)
			array.AddBackNogrowCrt(multiItemCreator);
		return array;
	}

	~Array() = default;

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
	MOMO_FRIENDS_SIZE_BEGIN_END_CONST(Array, ConstIterator)
	MOMO_FRIENDS_BEGIN_END(Array, Iterator)

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

	template<typename MultiItemCreator>
	void SetCountCrt(size_t count, const MultiItemCreator& multiItemCreator)
	{
		size_t newCount = count;
		size_t initCount = GetCount();
		size_t initCapacity = GetCapacity();
		if (newCount <= initCount)
		{
			pvRemoveBack(initCount - newCount);
		}
		else if (newCount <= initCapacity)
		{
			Item* items = GetItems();
			size_t index = initCount;
			try
			{
				for (; index < newCount; ++index)
					multiItemCreator(items + index);
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
			size_t newCapacity = pvGrowCapacity(initCapacity, newCount,
				ArrayGrowCause::reserve, false);
			auto itemsCreator = [this, initCount, newCount, &multiItemCreator] (Item* newItems)
			{
				size_t index = initCount;
				try
				{
					for (; index < newCount; ++index)
						multiItemCreator(newItems + index);
					ItemTraits::Relocate(GetMemManager(), GetItems(), newItems, initCount);
				}
				catch (...)
				{
					ItemTraits::Destroy(GetMemManager(), newItems + initCount, index - initCount);
					throw;
				}
			};
			mData.Reset(newCapacity, newCount, itemsCreator);
		}
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
			mData.Reset(capacity, count, itemsCreator);
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

	const Item& GetBackItem() const
	{
		return operator[](GetCount() - 1);
	}

	Item& GetBackItem()
	{
		return operator[](GetCount() - 1);
	}

	template<typename ItemCreator>
	void AddBackNogrowCrt(ItemCreator&& itemCreator)
	{
		MOMO_CHECK(GetCount() < GetCapacity());
		pvAddBackNogrow(std::forward<ItemCreator>(itemCreator));
	}

	template<typename... ItemArgs>
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

	template<typename ItemCreator>
	void AddBackCrt(ItemCreator&& itemCreator)
	{
		if (GetCount() < GetCapacity())
			pvAddBackNogrow(std::forward<ItemCreator>(itemCreator));
		else
			pvAddBackGrow(std::forward<ItemCreator>(itemCreator));
	}

	template<typename... ItemArgs>
	void AddBackVar(ItemArgs&&... itemArgs)
	{
		AddBackCrt(typename ItemTraits::template Creator<ItemArgs...>(GetMemManager(),
			std::forward<ItemArgs>(itemArgs)...));
	}

	void AddBack(Item&& item)
	{
		if (GetCount() < GetCapacity())
		{
			pvAddBackNogrow(typename ItemTraits::template Creator<Item&&>(
				GetMemManager(), std::move(item)));
		}
		else
		{
			pvAddBackGrow(std::move(item));
		}
	}

	void AddBack(const Item& item)
	{
		if (GetCount() < GetCapacity())
		{
			pvAddBackNogrow(typename ItemTraits::template Creator<const Item&>(
				GetMemManager(), item));
		}
		else
		{
			pvAddBackGrow(item);
		}
	}

	template<typename ItemCreator>
	void InsertCrt(size_t index, ItemCreator&& itemCreator)
	{
		ItemHandler itemHandler(GetMemManager(), std::forward<ItemCreator>(itemCreator));
		size_t newCount = GetCount() + 1;
		if (newCount > GetCapacity())
			pvGrow(newCount, ArrayGrowCause::add);
		ArrayShifter::InsertNogrow(*this, index, std::move(*&itemHandler));
	}

	template<typename... ItemArgs>
	void InsertVar(size_t index, ItemArgs&&... itemArgs)
	{
		InsertCrt(index, typename ItemTraits::template Creator<ItemArgs...>(GetMemManager(),
			std::forward<ItemArgs>(itemArgs)...));
	}

	void Insert(size_t index, Item&& item)
	{
		size_t initCount = GetCount();
		size_t grow = (initCount + 1 > GetCapacity());
		size_t itemIndex = pvIndexOf(item);
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
			MemManager& memManager = GetMemManager();
			ItemHandler itemHandler(memManager,
				typename ItemTraits::template Creator<const Item&>(memManager, item));
			if (grow)
				pvGrow(newCount, ArrayGrowCause::add);
			ArrayShifter::InsertNogrow(*this, index, count, *&itemHandler);
		}
		else
		{
			ArrayShifter::InsertNogrow(*this, index, count, item);
		}
	}

	template<typename ArgIterator, typename ArgSentinel,
		typename = decltype(*std::declval<ArgIterator>())>
	void Insert(size_t index, ArgIterator begin, ArgSentinel end)
	{
		MOMO_ASSERT(begin == end || !pvIsInside(*begin));	//?
		pvInsert(index, std::move(begin), std::move(end));
	}

	void Insert(size_t index, std::initializer_list<Item> items)
	{
		pvInsert(index, items.begin(), items.end());
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

	template<typename ItemFilter>
	internal::EnableIf<internal::IsInvocable<const ItemFilter&, bool, const Item&>::value,
	size_t> Remove(const ItemFilter& itemFilter)
	{
		return ArrayShifter::Remove(*this, itemFilter);
	}

	template<typename EqualFunc = std::equal_to<Item>>
	bool Contains(const Item& item, const EqualFunc& equalFunc = EqualFunc()) const
	{
		return std::any_of(GetBegin(), GetEnd(),
			[&item, &equalFunc] (const Item& thisItem) { return equalFunc(thisItem, item); });
	}

	template<typename EqualFunc = std::equal_to<Item>>
	bool IsEqual(const Array& array, const EqualFunc& equalFunc = EqualFunc()) const
	{
		return GetCount() == array.GetCount() &&
			std::equal(GetBegin(), GetEnd(), array.GetBegin(), equalFunc);
	}

private:
	explicit Array(Data&& data) noexcept
		: mData(std::move(data))
	{
	}

	template<typename ArgIterator, typename ArgSentinel>
	explicit Array(ArgIterator begin, ArgSentinel end, MemManager&& memManager,
		internal::EnableIf<internal::IsForwardIterator17<ArgIterator, ArgSentinel>::value, std::nullptr_t>)
		: mData(SMath::Dist(begin, end), std::move(memManager))
	{
		typedef typename ItemTraits::template Creator<decltype(*begin)> IterCreator;
		MemManager& thisMemManager = GetMemManager();
		for (ArgIterator iter = begin; iter != end; ++iter)
			AddBackNogrowCrt(IterCreator(thisMemManager, *iter));
	}

	template<typename ArgIterator, typename ArgSentinel>
	explicit Array(ArgIterator begin, ArgSentinel end, MemManager&& memManager,
		internal::EnableIf<!internal::IsForwardIterator17<ArgIterator, ArgSentinel>::value, std::nullptr_t>)
		: mData(std::move(memManager))
	{
		typedef typename ItemTraits::template Creator<decltype(*begin)> IterCreator;
		MemManager& thisMemManager = GetMemManager();
		for (ArgIterator iter = std::move(begin); iter != end; ++iter)
			AddBackCrt(IterCreator(thisMemManager, *iter));
	}

	static size_t pvGrowCapacity(size_t capacity, size_t minNewCapacity,
		ArrayGrowCause growCause, bool linear)
	{
		size_t newCapacity = Settings::GrowCapacity(capacity, minNewCapacity, growCause, linear);
		MOMO_ASSERT(newCapacity >= minNewCapacity);
		return newCapacity;
	}

	void pvGrow(size_t minNewCapacity, ArrayGrowCause growCause)
	{
		size_t initCapacity = GetCapacity();
		size_t newCapacityLin = pvGrowCapacity(initCapacity, minNewCapacity, growCause, true);
		size_t newCapacityExp = pvGrowCapacity(initCapacity, minNewCapacity, growCause, false);
		if (!mData.Reallocate(newCapacityLin, newCapacityExp))
		{
			size_t count = GetCount();
			auto itemsCreator = [this, count] (Item* newItems)
				{ ItemTraits::Relocate(GetMemManager(), GetItems(), newItems, count); };
			mData.Reset(newCapacityExp, count, itemsCreator);
		}
	}

	template<typename ItemCreator>
	void pvAddBackNogrow(ItemCreator&& itemCreator)
	{
		size_t count = GetCount();
		std::forward<ItemCreator>(itemCreator)(GetItems() + count);
		mData.SetCount(count + 1);
	}

	template<typename ItemCreator>
	void pvAddBackGrow(ItemCreator&& itemCreator)
	{
		size_t initCount = GetCount();
		size_t newCount = initCount + 1;
		size_t newCapacity = pvGrowCapacity(GetCapacity(), newCount, ArrayGrowCause::add, false);
		auto itemsCreator = [this, initCount, &itemCreator] (Item* newItems)
		{
			ItemTraits::RelocateCreate(GetMemManager(), GetItems(), newItems, initCount,
				std::forward<ItemCreator>(itemCreator), newItems + initCount);
		};
		mData.Reset(newCapacity, newCount, itemsCreator);
	}

	void pvAddBackGrow(Item&& item)
	{
		pvAddBackGrow(std::move(item),
			internal::BoolConstant<ItemTraits::isNothrowMoveConstructible>());
	}

	void pvAddBackGrow(Item&& item, std::true_type /*isNothrowMoveConstructible*/)
	{
		size_t initCount = GetCount();
		size_t newCount = initCount + 1;
		size_t itemIndex = pvIndexOf(static_cast<const Item&>(item));
		pvGrow(newCount, ArrayGrowCause::add);
		Item* items = GetItems();
		typename ItemTraits::template Creator<Item&&>(GetMemManager(),
			std::move((itemIndex == internal::UIntConst::maxSize)
				? item : items[itemIndex]))(items + initCount);
		mData.SetCount(newCount);
	}

	void pvAddBackGrow(Item&& item, std::false_type /*isNothrowMoveConstructible*/)
	{
		pvAddBackGrow(typename ItemTraits::template Creator<Item&&>(GetMemManager(),
			std::move(item)));
	}

	void pvAddBackGrow(const Item& item)
	{
		pvAddBackGrow(item,
			internal::BoolConstant<ItemTraits::isNothrowRelocatable>());
	}

	void pvAddBackGrow(const Item& item, std::true_type /*isNothrowRelocatable*/)
	{
		size_t initCount = GetCount();
		size_t newCount = initCount + 1;
		internal::ObjectBuffer<Item, ItemTraits::alignment> itemBuffer;
		MemManager& memManager = GetMemManager();
		typename ItemTraits::template Creator<const Item&>(memManager, item)(&itemBuffer);
		try
		{
			pvGrow(newCount, ArrayGrowCause::add);
		}
		catch (...)
		{
			ItemTraits::Destroy(memManager, &itemBuffer, 1);
			throw;
		}
		ItemTraits::Relocate(memManager, &itemBuffer, GetItems() + initCount, 1);
		mData.SetCount(newCount);
	}

	void pvAddBackGrow(const Item& item, std::false_type /*isNothrowRelocatable*/)
	{
		pvAddBackGrow(typename ItemTraits::template Creator<const Item&>(GetMemManager(), item));
	}

	template<typename ArgIterator, typename ArgSentinel>
	internal::EnableIf<internal::IsForwardIterator17<ArgIterator, ArgSentinel>::value>
	pvInsert(size_t index, ArgIterator begin, ArgSentinel end)
	{
		size_t count = SMath::Dist(begin, end);
		size_t newCount = GetCount() + count;
		if (newCount > GetCapacity())
			pvGrow(newCount, ArrayGrowCause::add);
		ArrayShifter::InsertNogrow(*this, index, begin, count);
	}

	template<typename ArgIterator, typename ArgSentinel>
	internal::EnableIf<!internal::IsForwardIterator17<ArgIterator, ArgSentinel>::value>
	pvInsert(size_t index, ArgIterator begin, ArgSentinel end)
	{
		ArrayShifter::Insert(*this, index, std::move(begin), std::move(end));
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

template<size_t tInternalCapacity, typename TItem,
	typename TMemManager = MemManagerDefault,
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

	template<size_t tInternalCapacity, typename TItem, typename TMemManager>
	using NestedArrayIntCap = Array<TItem, TMemManager, ArrayItemTraits<TItem, TMemManager>,
		NestedArraySettings<ArraySettings<tInternalCapacity>>>;
}

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_ARRAY

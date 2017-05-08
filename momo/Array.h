/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/Array.h

  namespace momo:
    class ArrayItemTraits
    enum class ArrayGrowCause
    struct ArraySettings
    class Array
    class ArrayIntCap

  All `Array` functions and constructors have strong exception safety,
  but not the following cases:
  1. Functions `Insert`, `InsertVar`, `InsertCrt`, `Remove` have
    basic exception safety.
  2. If any constructor throws exception, input argument `memManager`
    may be changed.

\**********************************************************/

#pragma once

#include "ArrayUtility.h"
#include "MemManager.h"

namespace momo
{

namespace internal
{
	template<typename TItem, typename TSettings>
	class ArrayPtrIterator
	{
	protected:
		typedef TItem Item;
		typedef TSettings Settings;

	public:
		typedef Item& Reference;
		typedef Item* Pointer;

		typedef ArrayPtrIterator<const Item, Settings> ConstIterator;

	public:
		explicit ArrayPtrIterator(Pointer pitem = nullptr) MOMO_NOEXCEPT
			: mItemPtr(pitem)
		{
		}

		template<typename Array>
		ArrayPtrIterator(Array* array, size_t index) MOMO_NOEXCEPT
			: mItemPtr(array->GetItems() + index)
		{
		}

		operator ConstIterator() const MOMO_NOEXCEPT
		{
			return ConstIterator(mItemPtr);
		}

		ArrayPtrIterator& operator+=(ptrdiff_t diff)
		{
			MOMO_CHECK(mItemPtr != nullptr || diff == 0);
			mItemPtr += diff;
			return *this;
		}

		ptrdiff_t operator-(ConstIterator iter) const
		{
			return mItemPtr - iter.GetItemPtr();
		}

		Pointer operator->() const
		{
			MOMO_CHECK(mItemPtr != nullptr);
			return mItemPtr;
		}

		bool operator==(ConstIterator iter) const MOMO_NOEXCEPT
		{
			return mItemPtr == iter.GetItemPtr();
		}

		bool operator<(ConstIterator iter) const
		{
			return std::less<const Item*>()(mItemPtr, iter.GetItemPtr());
			//return mItemPtr < iter.GetItemPtr();
		}

		MOMO_MORE_ARRAY_ITERATOR_OPERATORS(ArrayPtrIterator)

		Pointer GetItemPtr() const MOMO_NOEXCEPT
		{
			return mItemPtr;
		}

	private:
		Item* mItemPtr;
	};

	template<typename Array, bool usePtrIterator = Array::Settings::usePtrIterator>
	struct ArrayIteratorSelector;

	template<typename Array>
	struct ArrayIteratorSelector<Array, true>
	{
		typedef ArrayPtrIterator<typename Array::Item, typename Array::Settings> Iterator;
	};

	template<typename Array>
	struct ArrayIteratorSelector<Array, false>
	{
		typedef ArrayIndexIterator<Array, typename Array::Item> Iterator;
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
	static void Destroy(MemManager& memManager, Item* items, size_t count) MOMO_NOEXCEPT
	{
		ItemManager::Destroy(memManager, items, count);
	}

	template<typename ItemArg>
	static void Assign(MemManager& /*memManager*/, ItemArg&& itemArg, Item& item)
	{
		item = std::forward<ItemArg>(itemArg);
	}

	static void Relocate(MemManager& memManager, Item* srcItems, Item* dstItems, size_t count)
		MOMO_NOEXCEPT_IF(isNothrowRelocatable)
	{
		ItemManager::Relocate(memManager, srcItems, dstItems, count);
	}

	template<typename ItemCreator>
	static void RelocateCreate(MemManager& memManager, Item* srcItems, Item* dstItems,
		size_t count, const ItemCreator& itemCreator, Item* newItem)
	{
		ItemManager::RelocateCreate(memManager, srcItems, dstItems, count, itemCreator, newItem);
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
struct ArraySettings
{
	static const CheckMode checkMode = CheckMode::bydefault;

	static const size_t internalCapacity = tInternalCapacity;
	static const bool growOnReserve = tGrowOnReserve;
	static const bool usePtrIterator = tUsePtrIterator;

	static size_t GrowCapacity(size_t capacity, size_t minNewCapacity,
		ArrayGrowCause growCause, bool realloc)
	{
		MOMO_ASSERT(capacity < minNewCapacity);
		if (growCause == ArrayGrowCause::reserve && !growOnReserve)
			return minNewCapacity;
		size_t newCapacity;
		if (capacity <= 2)
			newCapacity = 4;
		else if (capacity < 256)
			newCapacity = capacity * 2;
		else if (capacity < 8192)
			newCapacity = capacity + capacity / 2;
		else if (realloc && capacity <= SIZE_MAX - 4096)
			newCapacity = capacity + 4096;
		else if (!realloc && capacity <= (SIZE_MAX / 146) * 100 + 99)
			newCapacity = (capacity / 100) * 146;	// k^4 < 1 + k + k^2
		else
			throw std::length_error("momo::ArraySettings length error");
		if (newCapacity < minNewCapacity)
			newCapacity = minNewCapacity;
		return newCapacity;
	}
};

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
	class Data : private internal::MemManagerWrapper<MemManager>
	{
	private:
		typedef internal::MemManagerWrapper<MemManager> MemManagerWrapper;

		static const bool hasInternalCapacity = (internalCapacity > 0);
		static const size_t maskInternal = hasInternalCapacity
			? (size_t)1 << (8 * sizeof(size_t) - 1) : 0;

		typedef internal::BoolConstant<hasInternalCapacity> HasInternalCapacity;

	public:
		explicit Data(MemManager&& memManager) MOMO_NOEXCEPT
			: MemManagerWrapper(std::move(memManager))
		{
			pvCreate(HasInternalCapacity());
		}

		explicit Data(size_t capacity, MemManager&& memManager = MemManager())
			: MemManagerWrapper(std::move(memManager))
		{
			pvCheckCapacity(capacity);
			if (capacity > internalCapacity)
			{
				mCount = 0;
				mExternalData.items = GetMemManager().template Allocate<Item>(capacity * sizeof(Item));
				mExternalData.capacity = capacity;
			}
			else
			{
				pvCreate(HasInternalCapacity());
			}
		}

		Data(Data&& data) MOMO_NOEXCEPT
			: MemManagerWrapper(std::move(data.pvGetMemManagerWrapper()))
		{
			pvCreateMove(std::move(data));
		}

		Data(const Data&) = delete;

		~Data() MOMO_NOEXCEPT
		{
			pvDestroy();
		}

		Data& operator=(Data&& data) MOMO_NOEXCEPT
		{
			if (this != &data)
			{
				pvDestroy();
				pvGetMemManagerWrapper() = std::move(data.pvGetMemManagerWrapper());
				pvCreateMove(std::move(data));
			}
			return *this;
		}

		Data& operator=(const Data&) = delete;

		const Item* GetItems() const MOMO_NOEXCEPT
		{
			return pvGetItems(&mInternalData, HasInternalCapacity());
		}

		Item* GetItems() MOMO_NOEXCEPT
		{
			return pvGetItems(&mInternalData, HasInternalCapacity());
		}

		const MemManager& GetMemManager() const MOMO_NOEXCEPT
		{
			return pvGetMemManagerWrapper().GetMemManager();
		}

		MemManager& GetMemManager() MOMO_NOEXCEPT
		{
			return pvGetMemManagerWrapper().GetMemManager();
		}

		size_t GetCapacity() const MOMO_NOEXCEPT
		{
			return pvIsInternal() ? internalCapacity : mExternalData.capacity;
		}

		bool SetCapacity(size_t capacity)
		{
			pvCheckCapacity(capacity);
			if (GetCapacity() == internalCapacity || capacity <= internalCapacity)
				return false;
			return pvSetCapacity(capacity,
				internal::BoolConstant<ItemTraits::isTriviallyRelocatable && MemManager::canReallocate>(),
				internal::BoolConstant<MemManager::canReallocateInplace>());
		}

		size_t GetCount() const MOMO_NOEXCEPT
		{
			return mCount & ~maskInternal;
		}

		void SetCount(size_t count) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(count <= GetCapacity());
			mCount &= maskInternal;
			mCount |= count;
		}

		void Clear() MOMO_NOEXCEPT
		{
			pvDestroy();
			pvCreate(HasInternalCapacity());
		}

		template<typename RelocateFunc>
		void Reset(size_t capacity, size_t count, RelocateFunc relocateFunc)
		{
			MOMO_ASSERT(count <= capacity);
			pvCheckCapacity(capacity);
			if (capacity > internalCapacity)
			{
				Item* items = GetMemManager().template Allocate<Item>(capacity * sizeof(Item));
				try
				{
					relocateFunc(items);
				}
				catch (...)
				{
					GetMemManager().Deallocate(items, capacity * sizeof(Item));
					throw;
				}
				pvDeallocate();
				mExternalData.items = items;
				mExternalData.capacity = capacity;
				mCount = count;
			}
			else
			{
				pvReset(count, relocateFunc, HasInternalCapacity());
			}
		}

	private:
		const MemManagerWrapper& pvGetMemManagerWrapper() const MOMO_NOEXCEPT
		{
			return *this;
		}

		MemManagerWrapper& pvGetMemManagerWrapper() MOMO_NOEXCEPT
		{
			return *this;
		}

		static void pvCheckCapacity(size_t capacity)
		{
			static const size_t maxCapacity = (sizeof(Item) > 1) ? SIZE_MAX / sizeof(Item)
				: (internalCapacity > 0) ? SIZE_MAX / 2 : SIZE_MAX - 1;
			if (capacity > maxCapacity)
				throw std::length_error("momo::Array length error");
		}

		void pvCreate(std::true_type /*hasInternalCapacity*/) MOMO_NOEXCEPT
		{
			mCount = maskInternal;
		}

		void pvCreate(std::false_type /*hasInternalCapacity*/) MOMO_NOEXCEPT
		{
			mCount = 0;
			mExternalData.items = nullptr;
			mExternalData.capacity = 0;
		}

		void pvCreateMove(Data&& data) MOMO_NOEXCEPT
		{
			pvMoveData(std::move(data), HasInternalCapacity());
			mCount = data.mCount;
			data.mCount = maskInternal;
		}

		void pvDestroy() MOMO_NOEXCEPT
		{
			ItemTraits::Destroy(GetMemManager(), GetItems(), GetCount());
			pvDeallocate();
		}

		void pvDeallocate() MOMO_NOEXCEPT
		{
			if (GetCapacity() > internalCapacity)
			{
				GetMemManager().Deallocate(mExternalData.items,
					mExternalData.capacity * sizeof(Item));
			}
		}

		void pvMoveData(Data&& data, std::true_type /*hasInternalCapacity*/) MOMO_NOEXCEPT
		{
			MOMO_STATIC_ASSERT(ItemTraits::isNothrowRelocatable);
			if (data.pvIsInternal())
				ItemTraits::Relocate(GetMemManager(), &data.mInternalData, &mInternalData, data.GetCount());
			else
				pvMoveData(std::move(data), std::false_type());
		}

		void pvMoveData(Data&& data, std::false_type /*hasInternalCapacity*/) MOMO_NOEXCEPT
		{
			mExternalData = data.mExternalData;
			data.mExternalData.items = nullptr;
			data.mExternalData.capacity = 0;
		}

		bool pvIsInternal() const MOMO_NOEXCEPT
		{
			return (mCount & maskInternal) != 0;
		}

		template<typename Item>
		Item* pvGetItems(Item* internalData,
			std::true_type /*hasInternalCapacity*/) const MOMO_NOEXCEPT
		{
			return pvIsInternal() ? internalData : mExternalData.items;
		}

		Item* pvGetItems(const void* /*internalData*/,
			std::false_type /*hasInternalCapacity*/) const MOMO_NOEXCEPT
		{
			return mExternalData.items;
		}

		template<bool canReallocateInplace>
		bool pvSetCapacity(size_t capacity, std::true_type /*canReallocate*/,
			internal::BoolConstant<canReallocateInplace>)
		{
			mExternalData.items = GetMemManager().template Reallocate<Item>(mExternalData.items,
				mExternalData.capacity * sizeof(Item), capacity * sizeof(Item));
			mExternalData.capacity = capacity;
			return true;
		}

		bool pvSetCapacity(size_t capacity, std::false_type /*canReallocate*/,
			std::true_type /*canReallocateInplace*/) MOMO_NOEXCEPT
		{
			bool reallocDone = GetMemManager().ReallocateInplace(mExternalData.items,
				mExternalData.capacity * sizeof(Item), capacity * sizeof(Item));
			if (!reallocDone)
				return false;
			mExternalData.capacity = capacity;
			return true;
		}

		bool pvSetCapacity(size_t /*capacity*/, std::false_type /*canReallocate*/,
			std::false_type /*canReallocateInplace*/) MOMO_NOEXCEPT
		{
			return false;
		}

		template<typename RelocateFunc>
		void pvReset(size_t count, RelocateFunc relocateFunc,
			std::true_type /*hasInternalCapacity*/)
		{
			MOMO_STATIC_ASSERT(ItemTraits::isNothrowRelocatable);
			internal::ArrayBuffer<ItemTraits, internalCapacity> internalData;
			relocateFunc(&internalData);
			pvDeallocate();
			ItemTraits::Relocate(GetMemManager(), &internalData, &mInternalData, count);
			mCount = maskInternal | count;
		}

		template<typename RelocateFunc>
		void pvReset(size_t count, RelocateFunc /*relocateFunc*/,
			std::false_type /*hasInternalCapacity*/) MOMO_NOEXCEPT
		{
			(void)count;
			MOMO_ASSERT(count == 0);
			pvDeallocate();
			pvCreate(std::false_type());
		}

	private:
		size_t mCount;
		union
		{
			struct
			{
				Item* items;
				size_t capacity;
			} mExternalData;
			internal::ArrayBuffer<ItemTraits, internalCapacity> mInternalData;
		};
	};

	typedef internal::ArrayItemHandler<ItemTraits> ItemHandler;
	typedef internal::ArrayShifter<Array> ArrayShifter;

public:
	typedef typename internal::ArrayIteratorSelector<Array>::Iterator Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

public:
	Array() MOMO_NOEXCEPT_IF(noexcept(MemManager()))
		: Array(MemManager())
	{
	}

	explicit Array(MemManager&& memManager) MOMO_NOEXCEPT
		: mData(std::move(memManager))
	{
	}

	explicit Array(size_t count, MemManager&& memManager = MemManager())
		: mData(count, std::move(memManager))
	{
		pvFill(count, typename ItemTraits::template Creator<>(GetMemManager()));
	}

	Array(size_t count, const Item& item, MemManager&& memManager = MemManager())
		: mData(count, std::move(memManager))
	{
		pvFill(count, typename ItemTraits::template Creator<const Item&>(GetMemManager(), item));
	}

	template<typename ArgIterator,
		typename = typename std::iterator_traits<ArgIterator>::iterator_category>
	Array(ArgIterator begin, ArgIterator end, MemManager&& memManager = MemManager())
		: mData(internal::IsForwardIterator<ArgIterator>::value ? std::distance(begin, end) : 0,
			std::move(memManager))
	{
		pvFill(begin, end);
	}

	Array(std::initializer_list<Item> items, MemManager&& memManager = MemManager())
		: Array(items.begin(), items.end(), std::move(memManager))
	{
	}

	Array(Array&& array) MOMO_NOEXCEPT
		: mData(std::move(array.mData))
	{
	}

	Array(const Array& array, bool shrink = true)
		: mData(shrink ? array.GetCount() : array.GetCapacity(), MemManager(array.GetMemManager()))
	{
		pvFill(array.GetBegin(), array.GetEnd());
	}

	Array(const Array& array, MemManager&& memManager)
		: Array(array.GetBegin(), array.GetEnd(), std::move(memManager))
	{
	}

	static Array CreateCap(size_t capacity, MemManager&& memManager = MemManager())
	{
		return Array(Data(capacity, std::move(memManager)));
	}

	template<typename ItemCreator>
	static Array CreateCrt(size_t count, const ItemCreator& itemCreator,
		MemManager&& memManager = MemManager())
	{
		Array array = CreateCap(count, std::move(memManager));
		array.pvFill(count, itemCreator);
		return array;
	}

	~Array() MOMO_NOEXCEPT
	{
	}

	Array& operator=(Array&& array) MOMO_NOEXCEPT
	{
		mData = std::move(array.mData);
		return *this;
	}

	Array& operator=(const Array& array)
	{
		if (this != &array)
			Array(array).Swap(*this);
		return *this;
	}

	void Swap(Array& array) MOMO_NOEXCEPT
	{
		std::swap(mData, array.mData);
	}

	ConstIterator GetBegin() const MOMO_NOEXCEPT
	{
		return ConstIterator(this, 0);
	}

	Iterator GetBegin() MOMO_NOEXCEPT
	{
		return Iterator(this, 0);
	}

	ConstIterator GetEnd() const MOMO_NOEXCEPT
	{
		return ConstIterator(this, GetCount());
	}

	Iterator GetEnd() MOMO_NOEXCEPT
	{
		return Iterator(this, GetCount());
	}

	MOMO_FRIEND_SWAP(Array)
	MOMO_FRIENDS_BEGIN_END(const Array&, ConstIterator)
	MOMO_FRIENDS_BEGIN_END(Array&, Iterator)

	const Item* GetItems() const MOMO_NOEXCEPT
	{
		return mData.GetItems();
	}

	Item* GetItems() MOMO_NOEXCEPT
	{
		return mData.GetItems();
	}

	const MemManager& GetMemManager() const MOMO_NOEXCEPT
	{
		return mData.GetMemManager();
	}

	MemManager& GetMemManager() MOMO_NOEXCEPT
	{
		return mData.GetMemManager();
	}

	size_t GetCount() const MOMO_NOEXCEPT
	{
		return mData.GetCount();
	}

	template<typename ItemCreator>
	void SetCountCrt(size_t count, const ItemCreator& itemCreator)
	{
		pvSetCount(count, itemCreator);
	}

	void SetCount(size_t count)
	{
		pvSetCount(count, typename ItemTraits::template Creator<>(GetMemManager()));
	}

	void SetCount(size_t count, const Item& item)
	{
		pvSetCount(count, typename ItemTraits::template Creator<const Item&>(GetMemManager(), item));
	}

	bool IsEmpty() const MOMO_NOEXCEPT
	{
		return GetCount() == 0;
	}

	void Clear(bool shrink = false) MOMO_NOEXCEPT
	{
		if (shrink)
			mData.Clear();
		else
			pvRemoveBack(GetCount());
	}

	size_t GetCapacity() const MOMO_NOEXCEPT
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
		size_t count = GetCount();
		size_t initCapacity = GetCapacity();
		if (initCapacity > count && initCapacity > internalCapacity)
		{
			size_t newCapacity = count;
			if (!mData.SetCapacity(newCapacity))
			{
				auto relocateFunc = [this, count] (Item* newItems)
					{ ItemTraits::Relocate(GetMemManager(), GetItems(), newItems, count); };
				mData.Reset(newCapacity, count, relocateFunc);
			}
		}
	}

	const Item& operator[](size_t index) const
	{
		return pvGetItem(GetItems(), index);
	}

	Item& operator[](size_t index)
	{
		return pvGetItem(GetItems(), index);
	}

	const Item& GetBackItem() const
	{
		return pvGetItem(GetItems(), GetCount() - 1);
	}

	Item& GetBackItem()
	{
		return pvGetItem(GetItems(), GetCount() - 1);
	}

	template<typename ItemCreator>
	void AddBackNogrowCrt(const ItemCreator& itemCreator)
	{
		MOMO_CHECK(GetCount() < GetCapacity());
		pvAddBackNogrow(itemCreator);
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
	void AddBackCrt(const ItemCreator& itemCreator)
	{
		if (GetCount() < GetCapacity())
			pvAddBackNogrow(itemCreator);
		else
			pvAddBackGrow(itemCreator);
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
			pvAddBackNogrow(typename ItemTraits::template Creator<Item>(
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
	void InsertCrt(size_t index, const ItemCreator& itemCreator)
	{
		ItemHandler itemHandler(GetMemManager(), itemCreator);
		std::move_iterator<Item*> begin(&itemHandler);
		Insert(index, begin, begin + 1);
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
		{
			InsertCrt(index, typename ItemTraits::template Creator<Item>(
				GetMemManager(), std::move(item)));
		}
		else
		{
			std::move_iterator<Item*> begin(std::addressof(item));
			ArrayShifter::Insert(*this, index, begin, begin + 1);
		}
	}

	void Insert(size_t index, const Item& item)
	{
		return Insert(index, (size_t)1, item);	//?
	}

	void Insert(size_t index, size_t count, const Item& item)
	{
		size_t initCount = GetCount();
		size_t newCount = initCount + count;
		size_t grow = (newCount > GetCapacity());
		size_t itemIndex = pvIndexOf(item);
		if (grow || (index <= itemIndex && itemIndex < initCount))
		{
			typename ItemTraits::template Creator<const Item&> itemCreator(GetMemManager(), item);
			ItemHandler itemHandler(GetMemManager(), itemCreator);
			if (grow)
				pvGrow(newCount, ArrayGrowCause::add);
			ArrayShifter::Insert(*this, index, count, *&itemHandler);
		}
		else
		{
			ArrayShifter::Insert(*this, index, count, item);
		}
	}

	template<typename ArgIterator,
		typename = typename std::iterator_traits<ArgIterator>::iterator_category>
	void Insert(size_t index, ArgIterator begin, ArgIterator end)
	{
		if (internal::IsForwardIterator<ArgIterator>::value)
		{
			size_t count = std::distance(begin, end);
			size_t newCount = GetCount() + count;
			if (newCount > GetCapacity())
				pvGrow(newCount, ArrayGrowCause::add);
		}
		ArrayShifter::Insert(*this, index, begin, end);
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

private:
	explicit Array(Data&& data) MOMO_NOEXCEPT
		: mData(std::move(data))
	{
	}

	template<typename ItemCreator>
	void pvFill(size_t count, const ItemCreator& itemCreator)
	{
		for (size_t i = 0; i < count; ++i)
			AddBackNogrowCrt(itemCreator);
	}

	template<typename ArgIterator>
	void pvFill(ArgIterator begin, ArgIterator end,
		typename std::enable_if<internal::IsForwardIterator<ArgIterator>::value, int>::type = 0)
	{
		typedef typename ItemTraits::template Creator<
			typename std::iterator_traits<ArgIterator>::reference> IterCreator;
		MemManager& memManager = GetMemManager();
		for (ArgIterator iter = begin; iter != end; ++iter)
			AddBackNogrowCrt(IterCreator(memManager, *iter));
	}

	template<typename ArgIterator>
	void pvFill(ArgIterator begin, ArgIterator end,
		typename std::enable_if<!internal::IsForwardIterator<ArgIterator>::value, int>::type = 0)
	{
		typedef typename ItemTraits::template Creator<
			typename std::iterator_traits<ArgIterator>::reference> IterCreator;
		MemManager& memManager = GetMemManager();
		for (ArgIterator iter = begin; iter != end; ++iter)
			AddBackCrt(IterCreator(memManager, *iter));
	}

	static size_t pvGrowCapacity(size_t capacity, size_t minNewCapacity,
		ArrayGrowCause growCause, bool realloc)
	{
		size_t newCapacity = Settings::GrowCapacity(capacity, minNewCapacity, growCause, realloc);
		MOMO_ASSERT(newCapacity >= minNewCapacity);
		return newCapacity;
	}

	void pvGrow(size_t minNewCapacity, ArrayGrowCause growCause)
	{
		size_t initCapacity = GetCapacity();
		if (!mData.SetCapacity(pvGrowCapacity(initCapacity, minNewCapacity, growCause, true)))
		{
			size_t count = GetCount();
			auto relocateFunc = [this, count] (Item* newItems)
				{ ItemTraits::Relocate(GetMemManager(), GetItems(), newItems, count); };
			mData.Reset(pvGrowCapacity(initCapacity, minNewCapacity, growCause, false),
				count, relocateFunc);
		}
	}

	template<typename ItemCreator>
	void pvSetCount(size_t count, const ItemCreator& itemCreator)
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
					itemCreator(items + index);
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
			auto relocateFunc = [this, initCount, newCount, &itemCreator] (Item* newItems)
			{
				size_t index = initCount;
				try
				{
					for (; index < newCount; ++index)
						itemCreator(newItems + index);
					ItemTraits::Relocate(GetMemManager(), GetItems(), newItems, initCount);
				}
				catch (...)
				{
					ItemTraits::Destroy(GetMemManager(), newItems + initCount, index - initCount);
					throw;
				}
			};
			mData.Reset(newCapacity, newCount, relocateFunc);
		}
	}

	template<typename Item>
	Item& pvGetItem(Item* items, size_t index) const
	{
		MOMO_CHECK(index < GetCount());
		return items[index];
	}

	template<typename ItemCreator>
	void pvAddBackNogrow(const ItemCreator& itemCreator)
	{
		size_t count = GetCount();
		itemCreator(GetItems() + count);
		mData.SetCount(count + 1);
	}

	template<typename ItemCreator>
	void pvAddBackGrow(const ItemCreator& itemCreator)
	{
		size_t initCount = GetCount();
		size_t newCount = initCount + 1;
		size_t newCapacity = pvGrowCapacity(GetCapacity(), newCount, ArrayGrowCause::add, false);
		auto relocateFunc = [this, initCount, &itemCreator] (Item* newItems)
		{
			ItemTraits::RelocateCreate(GetMemManager(), GetItems(), newItems, initCount,
				itemCreator, newItems + initCount);
		};
		mData.Reset(newCapacity, newCount, relocateFunc);
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
		typename ItemTraits::template Creator<Item>(GetMemManager(),
			std::move(itemIndex == SIZE_MAX ? item : items[itemIndex]))(items + initCount);
		mData.SetCount(newCount);
	}

	void pvAddBackGrow(Item&& item, std::false_type /*isNothrowMoveConstructible*/)
	{
		pvAddBackGrow(typename ItemTraits::template Creator<Item>(GetMemManager(), std::move(item)));
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
		typename ItemTraits::template Creator<const Item&>(GetMemManager(), item)(&itemBuffer);
		try
		{
			pvGrow(newCount, ArrayGrowCause::add);
		}
		catch (...)
		{
			ItemTraits::Destroy(GetMemManager(), &itemBuffer, 1);
			throw;
		}
		ItemTraits::Relocate(GetMemManager(), &itemBuffer, GetItems() + initCount, 1);
		mData.SetCount(newCount);
	}

	void pvAddBackGrow(const Item& item, std::false_type /*isNothrowRelocatable*/)
	{
		pvAddBackGrow(typename ItemTraits::template Creator<const Item&>(GetMemManager(), item));
	}

	void pvRemoveBack(size_t count) MOMO_NOEXCEPT
	{
		size_t initCount = GetCount();
		ItemTraits::Destroy(GetMemManager(), GetItems() + initCount - count, count);
		mData.SetCount(initCount - count);
	}

	size_t pvIndexOf(const Item& item) const MOMO_NOEXCEPT
	{
		const Item* pitem = std::addressof(item);
		const Item* items = GetItems();
		std::less<const Item*> less;
		return (!less(pitem, items) && less(pitem, items + GetCount())) ? pitem - items : SIZE_MAX;
	}

private:
	Data mData;
};

template<size_t tInternalCapacity, typename TItem,
	typename TMemManager = MemManagerDefault,
	typename TItemTraits = ArrayItemTraits<TItem, TMemManager>,
	typename TSettings = ArraySettings<tInternalCapacity>>
using ArrayIntCap = Array<TItem, TMemManager, TItemTraits, TSettings>;

namespace internal
{
	template<typename TBaseArraySettings = ArraySettings<>>
	struct NestedArraySettings : public TBaseArraySettings
	{
	protected:
		typedef TBaseArraySettings BaseArraySettings;

	public:
		static const CheckMode checkMode = CheckMode::assertion;
	};

	template<size_t internalCapacity, typename Item, typename MemManager>
	using NestedArrayIntCap = Array<Item, MemManager, ArrayItemTraits<Item, MemManager>,
		NestedArraySettings<ArraySettings<internalCapacity>>>;
}

} // namespace momo

namespace std
{
	template<typename I, typename S>
	struct iterator_traits<momo::internal::ArrayPtrIterator<I, S>>
		: public iterator_traits<I*>
	{
	};
} // namespace std

/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/Array.h

  namespace momo:
    struct ArrayItemTraits
    enum class ArrayGrowCause
    struct ArraySettings
    class Array

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
	public:
		typedef TItem Item;
		typedef TSettings Settings;

		typedef Item& Reference;
		typedef Item* Pointer;

		typedef ArrayPtrIterator<const Item, Settings> ConstIterator;

	public:
		explicit ArrayPtrIterator(Item* pitem = nullptr) MOMO_NOEXCEPT
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
			MOMO_CHECK(mItemPtr != nullptr);
			mItemPtr += diff;
			return *this;
		}

		ptrdiff_t operator-(ConstIterator iter) const
		{
			return mItemPtr - iter.GetItemPtr();
		}

		Item& operator*() const
		{
			MOMO_CHECK(mItemPtr != nullptr);
			return *mItemPtr;
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

		Item* GetItemPtr() const MOMO_NOEXCEPT
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
		typedef ArrayIterator<Array, typename Array::Item> Iterator;
	};

	template<typename TArraySettings>
	struct NestedArraySettings : public TArraySettings
	{
		typedef TArraySettings ArraySettings;

		static const CheckMode checkMode = CheckMode::assertion;
	};
}

template<typename TItem>
struct ArrayItemTraits
{
	typedef TItem Item;

	typedef internal::ObjectManager<Item> ItemManager;

	static const size_t alignment = ItemManager::alignment;

	static const bool isNothrowMoveConstructible = ItemManager::isNothrowMoveConstructible;
	static const bool isTriviallyRelocatable = ItemManager::isTriviallyRelocatable;
	static const bool isNothrowRelocatable = ItemManager::isNothrowRelocatable;

	template<typename... ItemArgs>
	using Creator = typename ItemManager::template Creator<ItemArgs...>;

	static void Destroy(Item* items, size_t count) MOMO_NOEXCEPT
	{
		ItemManager::Destroy(items, count);
	}

	template<typename ItemArg>
	static void Assign(ItemArg&& itemArg, Item& item)
	{
		item = std::forward<ItemArg>(itemArg);
	}

	static void Relocate(Item* srcItems, Item* dstItems, size_t count)
		MOMO_NOEXCEPT_IF(isNothrowRelocatable)
	{
		ItemManager::Relocate(srcItems, dstItems, count);
	}

	template<typename ItemCreator>
	static void RelocateCreate(Item* srcItems, Item* dstItems, size_t count,
		const ItemCreator& itemCreator, void* pitem)
	{
		ItemManager::RelocateCreate(srcItems, dstItems, count, itemCreator, pitem);
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
	typename TItemTraits = ArrayItemTraits<TItem>,
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
			_Create(HasInternalCapacity());
		}

		explicit Data(size_t capacity, MemManager&& memManager = MemManager())
			: MemManagerWrapper(std::move(memManager))
		{
			_CheckCapacity(capacity);
			if (capacity > internalCapacity)
			{
				mCount = 0;
				mExternalData.items = GetMemManager().template Allocate<Item>(capacity * sizeof(Item));
				mExternalData.capacity = capacity;
			}
			else
			{
				_Create(HasInternalCapacity());
			}
		}

		Data(Data&& data) MOMO_NOEXCEPT
			: MemManagerWrapper(std::move(data._GetMemManagerWrapper()))
		{
			_CreateMove(std::move(data));
		}

		Data(const Data&) = delete;

		~Data() MOMO_NOEXCEPT
		{
			_Destroy();
		}

		Data& operator=(Data&& data) MOMO_NOEXCEPT
		{
			if (this != &data)
			{
				_Destroy();
				_GetMemManagerWrapper() = std::move(data._GetMemManagerWrapper());
				_CreateMove(std::move(data));
			}
			return *this;
		}

		Data& operator=(const Data&) = delete;

		const Item* GetItems() const MOMO_NOEXCEPT
		{
			return _GetItems(&mInternalData, HasInternalCapacity());
		}

		Item* GetItems() MOMO_NOEXCEPT
		{
			return _GetItems(&mInternalData, HasInternalCapacity());
		}

		const MemManager& GetMemManager() const MOMO_NOEXCEPT
		{
			return _GetMemManagerWrapper().GetMemManager();
		}

		MemManager& GetMemManager() MOMO_NOEXCEPT
		{
			return _GetMemManagerWrapper().GetMemManager();
		}

		size_t GetCapacity() const MOMO_NOEXCEPT
		{
			return _IsInternal() ? internalCapacity : mExternalData.capacity;
		}

		bool SetCapacity(size_t capacity)
		{
			_CheckCapacity(capacity);
			if (GetCapacity() == internalCapacity || capacity <= internalCapacity)
				return false;
			return _SetCapacity(capacity,
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
			_Destroy();
			_Create(HasInternalCapacity());
		}

		template<typename RelocateFunc>
		void Reset(size_t capacity, size_t count, RelocateFunc relocateFunc)
		{
			MOMO_ASSERT(count <= capacity);
			_CheckCapacity(capacity);
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
				_Deallocate();
				mExternalData.items = items;
				mExternalData.capacity = capacity;
				mCount = count;
			}
			else
			{
				_Reset(count, relocateFunc, HasInternalCapacity());
			}
		}

	private:
		const MemManagerWrapper& _GetMemManagerWrapper() const MOMO_NOEXCEPT
		{
			return *this;
		}

		MemManagerWrapper& _GetMemManagerWrapper() MOMO_NOEXCEPT
		{
			return *this;
		}

		static void _CheckCapacity(size_t capacity)
		{
			static const size_t maxCapacity = (sizeof(Item) > 1) ? SIZE_MAX / sizeof(Item)
				: (internalCapacity > 0) ? SIZE_MAX / 2 : SIZE_MAX - 1;
			if (capacity > maxCapacity)
				throw std::length_error("momo::Array length error");
		}

		void _Create(std::true_type /*hasInternalCapacity*/) MOMO_NOEXCEPT
		{
			mCount = maskInternal;
		}

		void _Create(std::false_type /*hasInternalCapacity*/) MOMO_NOEXCEPT
		{
			mCount = 0;
			mExternalData.items = nullptr;
			mExternalData.capacity = 0;
		}

		void _CreateMove(Data&& data) MOMO_NOEXCEPT
		{
			_MoveData(std::move(data), HasInternalCapacity());
			mCount = data.mCount;
			data.mCount = maskInternal;
		}

		void _Destroy() MOMO_NOEXCEPT
		{
			ItemTraits::Destroy(GetItems(), GetCount());
			_Deallocate();
		}

		void _Deallocate() MOMO_NOEXCEPT
		{
			if (GetCapacity() > internalCapacity)
			{
				GetMemManager().Deallocate(mExternalData.items,
					mExternalData.capacity * sizeof(Item));
			}
		}

		void _MoveData(Data&& data, std::true_type /*hasInternalCapacity*/) MOMO_NOEXCEPT
		{
			MOMO_STATIC_ASSERT(ItemTraits::isNothrowRelocatable);
			if (data._IsInternal())
				ItemTraits::Relocate(&data.mInternalData, &mInternalData, data.GetCount());
			else
				_MoveData(std::move(data), std::false_type());
		}

		void _MoveData(Data&& data, std::false_type /*hasInternalCapacity*/) MOMO_NOEXCEPT
		{
			mExternalData = data.mExternalData;
			data.mExternalData.items = nullptr;
			data.mExternalData.capacity = 0;
		}

		bool _IsInternal() const MOMO_NOEXCEPT
		{
			return (mCount & maskInternal) != 0;
		}

		template<typename Item>
		Item* _GetItems(Item* internalData,
			std::true_type /*hasInternalCapacity*/) const MOMO_NOEXCEPT
		{
			return _IsInternal() ? internalData : mExternalData.items;
		}

		Item* _GetItems(const void* /*internalData*/,
			std::false_type /*hasInternalCapacity*/) const MOMO_NOEXCEPT
		{
			return mExternalData.items;
		}

		template<bool canReallocateInplace>
		bool _SetCapacity(size_t capacity, std::true_type /*canReallocate*/,
			internal::BoolConstant<canReallocateInplace>)
		{
			mExternalData.items = GetMemManager().template Reallocate<Item>(mExternalData.items,
				mExternalData.capacity * sizeof(Item), capacity * sizeof(Item));
			mExternalData.capacity = capacity;
			return true;
		}

		bool _SetCapacity(size_t capacity, std::false_type /*canReallocate*/,
			std::true_type /*canReallocateInplace*/) MOMO_NOEXCEPT
		{
			bool reallocDone = GetMemManager().ReallocateInplace(mExternalData.items,
				mExternalData.capacity * sizeof(Item), capacity * sizeof(Item));
			if (!reallocDone)
				return false;
			mExternalData.capacity = capacity;
			return true;
		}

		bool _SetCapacity(size_t /*capacity*/, std::false_type /*canReallocate*/,
			std::false_type /*canReallocateInplace*/) MOMO_NOEXCEPT
		{
			return false;
		}

		template<typename RelocateFunc>
		void _Reset(size_t count, RelocateFunc relocateFunc,
			std::true_type /*hasInternalCapacity*/)
		{
			MOMO_STATIC_ASSERT(ItemTraits::isNothrowRelocatable);
			internal::ArrayBuffer<ItemTraits, internalCapacity> internalData;
			relocateFunc(&internalData);
			_Deallocate();
			ItemTraits::Relocate(&internalData, &mInternalData, count);
			mCount = maskInternal | count;
		}

		template<typename RelocateFunc>
		void _Reset(size_t count, RelocateFunc /*relocateFunc*/,
			std::false_type /*hasInternalCapacity*/) MOMO_NOEXCEPT
		{
			(void)count;
			MOMO_ASSERT(count == 0);
			_Deallocate();
			_Create(std::false_type());
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
	Array()
		: Array(MemManager())
	{
	}

	explicit Array(MemManager&& memManager) MOMO_NOEXCEPT
		: mData(std::move(memManager))
	{
	}

	explicit Array(size_t count, MemManager&& memManager = MemManager())
		: mData(_CreateData(count, typename ItemTraits::template Creator<>(),
			std::move(memManager)))
	{
	}

	Array(size_t count, const Item& item, MemManager&& memManager = MemManager())
		: mData(_CreateData(count, typename ItemTraits::template Creator<const Item&>(item),
			std::move(memManager)))
	{
	}

	template<typename Iterator>
	Array(Iterator begin, Iterator end, MemManager&& memManager = MemManager())
		: mData(internal::IsForwardIterator<Iterator>::value ? std::distance(begin, end) : 0,
			std::move(memManager))
	{
		_Fill(begin, end, internal::IsForwardIterator<Iterator>());
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
		_Fill(array.GetBegin(), array.GetEnd(), std::true_type());
	}

	static Array CreateCap(size_t capacity, MemManager&& memManager = MemManager())
	{
		return Array(Data(capacity, std::move(memManager)));
	}

	template<typename ItemCreator>
	static Array CreateCrt(size_t count, const ItemCreator& itemCreator,
		MemManager&& memManager = MemManager())
	{
		return Array(_CreateData(count, itemCreator, std::move(memManager)));
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
		_SetCount(count, itemCreator);
	}

	void SetCount(size_t count)
	{
		_SetCount(count, typename ItemTraits::template Creator<>());
	}

	void SetCount(size_t count, const Item& item)
	{
		_SetCount(count, typename ItemTraits::template Creator<const Item&>(item));
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
			_RemoveBack(GetCount());
	}

	size_t GetCapacity() const MOMO_NOEXCEPT
	{
		return mData.GetCapacity();
	}

	void Reserve(size_t capacity)
	{
		if (capacity > GetCapacity())
			_Grow(capacity, ArrayGrowCause::reserve);
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
				Item* items = GetItems();
				auto relocateFunc = [items, count] (Item* newItems)
					{ ItemTraits::Relocate(items, newItems, count); };
				mData.Reset(newCapacity, count, relocateFunc);
			}
		}
	}

	const Item& operator[](size_t index) const
	{
		return _GetItem(GetItems(), index);
	}

	Item& operator[](size_t index)
	{
		return _GetItem(GetItems(), index);
	}

	const Item& GetBackItem() const
	{
		return _GetItem(GetItems(), GetCount() - 1);
	}

	Item& GetBackItem()
	{
		return _GetItem(GetItems(), GetCount() - 1);
	}

	template<typename ItemCreator>
	void AddBackNogrowCrt(const ItemCreator& itemCreator)
	{
		MOMO_CHECK(GetCount() < GetCapacity());
		_AddBackNogrow(itemCreator);
	}

	template<typename... ItemArgs>
	void AddBackNogrowVar(ItemArgs&&... itemArgs)
	{
		AddBackNogrowCrt(typename ItemTraits::template Creator<ItemArgs...>(
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
			_AddBackNogrow(itemCreator);
		else
			_AddBackGrow(itemCreator);
	}

	template<typename... ItemArgs>
	void AddBackVar(ItemArgs&&... itemArgs)
	{
		AddBackCrt(typename ItemTraits::template Creator<ItemArgs...>(
			std::forward<ItemArgs>(itemArgs)...));
	}

	void AddBack(Item&& item)
	{
		if (GetCount() < GetCapacity())
			_AddBackNogrow(typename ItemTraits::template Creator<Item>(std::move(item)));
		else
			_AddBackGrow(std::move(item));
	}

	void AddBack(const Item& item)
	{
		if (GetCount() < GetCapacity())
			_AddBackNogrow(typename ItemTraits::template Creator<const Item&>(item));
		else
			_AddBackGrow(item);
	}

	template<typename ItemCreator>
	void InsertCrt(size_t index, const ItemCreator& itemCreator)
	{
		ItemHandler itemHandler(itemCreator);
		std::move_iterator<Item*> begin(&itemHandler);
		Insert(index, begin, begin + 1);
	}

	template<typename... ItemArgs>
	void InsertVar(size_t index, ItemArgs&&... itemArgs)
	{
		InsertCrt(index, typename ItemTraits::template Creator<ItemArgs...>(
			std::forward<ItemArgs>(itemArgs)...));
	}

	void Insert(size_t index, Item&& item)
	{
		size_t initCount = GetCount();
		size_t grow = (initCount + 1 > GetCapacity());
		size_t itemIndex = _IndexOf(item);
		if (grow || (index <= itemIndex && itemIndex < initCount))
		{
			InsertCrt(index, typename ItemTraits::template Creator<Item>(std::move(item)));
		}
		else
		{
			std::move_iterator<Item*> begin(std::addressof(item));
			ArrayShifter::Insert(*this, index, begin, begin + 1,
				internal::IsForwardIterator<Iterator>());
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
		size_t itemIndex = _IndexOf(item);
		if (grow || (index <= itemIndex && itemIndex < initCount))
		{
			typename ItemTraits::template Creator<const Item&> itemCreator(item);
			ItemHandler itemHandler(itemCreator);
			if (grow)
				_Grow(newCount, ArrayGrowCause::add);
			ArrayShifter::Insert(*this, index, count, *&itemHandler);
		}
		else
		{
			ArrayShifter::Insert(*this, index, count, item);
		}
	}

	template<typename Iterator>
	void Insert(size_t index, Iterator begin, Iterator end)
	{
		if (internal::IsForwardIterator<Iterator>::value)
		{
			size_t count = std::distance(begin, end);
			size_t newCount = GetCount() + count;
			if (newCount > GetCapacity())
				_Grow(newCount, ArrayGrowCause::add);
		}
		ArrayShifter::Insert(*this, index, begin, end, internal::IsForwardIterator<Iterator>());
	}

	void Insert(size_t index, std::initializer_list<Item> items)
	{
		Insert(index, items.begin(), items.end());
	}

	void RemoveBack(size_t count = 1)
	{
		MOMO_CHECK(count <= GetCount());
		_RemoveBack(count);
	}

	void Remove(size_t index, size_t count)
	{
		ArrayShifter::Remove(*this, index, count);
	}

private:
	explicit Array(Data&& data) MOMO_NOEXCEPT
		: mData(std::move(data))
	{
	}

	template<typename ItemCreator>
	static Data _CreateData(size_t count, const ItemCreator& itemCreator,
		MemManager&& memManager)
	{
		Data data(count, std::move(memManager));
		Item* items = data.GetItems();
		for (size_t i = 0; i < count; ++i)
		{
			itemCreator(items + i);
			data.SetCount(i + 1);
		}
		return data;
	}

	template<typename Iterator>
	void _Fill(Iterator begin, Iterator end, std::true_type /*isForwardIterator*/)
	{
		typedef typename ItemTraits::template Creator<
			typename std::iterator_traits<Iterator>::reference> IterCreator;
		for (Iterator iter = begin; iter != end; ++iter)
			AddBackNogrowCrt(IterCreator(*iter));
	}

	template<typename Iterator>
	void _Fill(Iterator begin, Iterator end, std::false_type /*isForwardIterator*/)
	{
		typedef typename ItemTraits::template Creator<
			typename std::iterator_traits<Iterator>::reference> IterCreator;
		for (Iterator iter = begin; iter != end; ++iter)
			AddBackCrt(IterCreator(*iter));
	}

	static size_t _GrowCapacity(size_t capacity, size_t minNewCapacity,
		ArrayGrowCause growCause, bool realloc)
	{
		size_t newCapacity = Settings::GrowCapacity(capacity, minNewCapacity, growCause, realloc);
		MOMO_ASSERT(newCapacity >= minNewCapacity);
		return newCapacity;
	}

	void _Grow(size_t minNewCapacity, ArrayGrowCause growCause)
	{
		size_t initCapacity = GetCapacity();
		if (!mData.SetCapacity(_GrowCapacity(initCapacity, minNewCapacity, growCause, true)))
		{
			Item* items = GetItems();
			size_t count = GetCount();
			auto relocateFunc = [items, count] (Item* newItems)
				{ ItemTraits::Relocate(items, newItems, count); };
			mData.Reset(_GrowCapacity(initCapacity, minNewCapacity, growCause, false),
				count, relocateFunc);
		}
	}

	template<typename ItemCreator>
	void _SetCount(size_t count, const ItemCreator& itemCreator)
	{
		size_t newCount = count;
		size_t initCount = GetCount();
		size_t initCapacity = GetCapacity();
		if (newCount <= initCount)
		{
			_RemoveBack(initCount - newCount);
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
				ItemTraits::Destroy(items + initCount, index - initCount);
				throw;
			}
			mData.SetCount(newCount);
		}
		else
		{
			size_t newCapacity = _GrowCapacity(initCapacity, newCount,
				ArrayGrowCause::reserve, false);
			Item* items = GetItems();
			auto relocateFunc = [items, initCount, newCount, &itemCreator] (Item* newItems)
			{
				size_t index = initCount;
				try
				{
					for (; index < newCount; ++index)
						itemCreator(newItems + index);
					ItemTraits::Relocate(items, newItems, initCount);
				}
				catch (...)
				{
					ItemTraits::Destroy(newItems + initCount, index - initCount);
					throw;
				}
			};
			mData.Reset(newCapacity, newCount, relocateFunc);
		}
	}

	template<typename Item>
	Item& _GetItem(Item* items, size_t index) const
	{
		MOMO_CHECK(index < GetCount());
		return items[index];
	}

	template<typename ItemCreator>
	void _AddBackNogrow(const ItemCreator& itemCreator)
	{
		size_t count = GetCount();
		itemCreator(GetItems() + count);
		mData.SetCount(count + 1);
	}

	template<typename ItemCreator>
	void _AddBackGrow(const ItemCreator& itemCreator)
	{
		size_t initCount = GetCount();
		size_t newCount = initCount + 1;
		size_t newCapacity = _GrowCapacity(GetCapacity(), newCount, ArrayGrowCause::add, false);
		Item* items = GetItems();
		auto relocateFunc = [items, initCount, &itemCreator] (Item* newItems)
		{
			ItemTraits::RelocateCreate(items, newItems, initCount,
				itemCreator, newItems + initCount);
		};
		mData.Reset(newCapacity, newCount, relocateFunc);
	}

	void _AddBackGrow(Item&& item)
	{
		_AddBackGrow(std::move(item),
			internal::BoolConstant<ItemTraits::isNothrowMoveConstructible>());
	}

	void _AddBackGrow(Item&& item, std::true_type /*isNothrowMoveConstructible*/)
	{
		size_t initCount = GetCount();
		size_t newCount = initCount + 1;
		size_t itemIndex = _IndexOf(static_cast<const Item&>(item));
		_Grow(newCount, ArrayGrowCause::add);
		Item* items = GetItems();
		typename ItemTraits::template Creator<Item>
			(std::move(itemIndex == SIZE_MAX ? item : items[itemIndex]))(items + initCount);
		mData.SetCount(newCount);
	}

	void _AddBackGrow(Item&& item, std::false_type /*isNothrowMoveConstructible*/)
	{
		_AddBackGrow(typename ItemTraits::template Creator<Item>(std::move(item)));
	}

	void _AddBackGrow(const Item& item)
	{
		_AddBackGrow(item,
			internal::BoolConstant<ItemTraits::isNothrowRelocatable>());
	}

	void _AddBackGrow(const Item& item, std::true_type /*isNothrowRelocatable*/)
	{
		size_t initCount = GetCount();
		size_t newCount = initCount + 1;
		internal::ObjectBuffer<Item, ItemTraits::alignment> itemBuffer;
		(typename ItemTraits::template Creator<const Item&>(item))(&itemBuffer);
		try
		{
			_Grow(newCount, ArrayGrowCause::add);
		}
		catch (...)
		{
			ItemTraits::Destroy(&itemBuffer, 1);
			throw;
		}
		ItemTraits::Relocate(&itemBuffer, GetItems() + initCount, 1);
		mData.SetCount(newCount);
	}

	void _AddBackGrow(const Item& item, std::false_type /*isNothrowRelocatable*/)
	{
		_AddBackGrow(typename ItemTraits::template Creator<const Item&>(item));
	}

	void _RemoveBack(size_t count) MOMO_NOEXCEPT
	{
		size_t initCount = GetCount();
		ItemTraits::Destroy(GetItems() + initCount - count, count);
		mData.SetCount(initCount - count);
	}

	size_t _IndexOf(const Item& item) const MOMO_NOEXCEPT
	{
		const Item* pitem = std::addressof(item);
		const Item* items = GetItems();
		std::less<const Item*> less;
		return (!less(pitem, items) && less(pitem, items + GetCount())) ? pitem - items : SIZE_MAX;
	}

private:
	Data mData;
};

} // namespace momo

namespace std
{
	template<typename I, typename S>
	struct iterator_traits<momo::internal::ArrayPtrIterator<I, S>>
		: public iterator_traits<I*>
	{
	};
} // namespace std

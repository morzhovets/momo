/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/Array.h

  namespace momo:
    class ArrayItemTraits
    enum class ArrayGrowCause
    class ArraySettings
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
		explicit ArrayPtrIterator(Item* pitem = nullptr) noexcept
			: mItemPtr(pitem)
		{
		}

		operator ConstIterator() const noexcept
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

		bool operator==(ConstIterator iter) const noexcept
		{
			return mItemPtr == iter.GetItemPtr();
		}

		bool operator<(ConstIterator iter) const
		{
			return std::less<const Item*>()(mItemPtr, iter.GetItemPtr());
			//return mItemPtr < iter.GetItemPtr();
		}

		MOMO_MORE_ARRAY_ITERATOR_OPERATORS(ArrayPtrIterator)

		Item* GetItemPtr() const noexcept
		{
			return mItemPtr;
		}

	private:
		Item* mItemPtr;
	};

	template<typename Array,
		bool usePtrIterator = Array::Settings::usePtrIterator>
	class ArrayIteratorSelector;

	template<typename Array>
	class ArrayIteratorSelector<Array, true>
	{
	public:
		typedef ArrayPtrIterator<typename Array::Item, typename Array::Settings> Iterator;
		typedef typename Iterator::ConstIterator ConstIterator;

	public:
		static ConstIterator MakeIterator(const Array& array, size_t index) noexcept
		{
			return ConstIterator(array.GetItems() + index);
		}

		static Iterator MakeIterator(Array& array, size_t index) noexcept
		{
			return Iterator(array.GetItems() + index);
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

	template<typename ItemArg>
	static void Assign(MemManager& /*memManager*/, ItemArg&& itemArg, Item& item)
	{
		item = std::forward<ItemArg>(itemArg);
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
	class Data : private MemManager
	{
	private:
		typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

	public:
		explicit Data(MemManager&& memManager) noexcept
			: MemManager(std::move(memManager))
		{
			pvCreate();
		}

		explicit Data(size_t capacity, MemManager&& memManager)
			: MemManager(std::move(memManager))
		{
			pvCheckCapacity(capacity);
			if (capacity > internalCapacity)
			{
				mItems = MemManagerProxy::template Allocate<Item>(GetMemManager(),
					capacity * sizeof(Item));
				mCount = 0;
				mCapacity = capacity;
			}
			else
			{
				pvCreate();
			}
		}

		Data(Data&& data) noexcept
			: MemManager(std::move(data.GetMemManager()))
		{
			pvCreateMove(std::move(data));
		}

		Data(const Data&) = delete;

		~Data() noexcept
		{
			pvDestroy();
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

		bool SetCapacity(size_t capacity)
		{
			pvCheckCapacity(capacity);
			if (GetCapacity() == internalCapacity || capacity <= internalCapacity)
				return false;
			static const bool canReallocate = ItemTraits::isTriviallyRelocatable
				&& MemManagerProxy::canReallocate;
			return pvSetCapacity(capacity, internal::BoolConstant<canReallocate>(),
				internal::BoolConstant<MemManagerProxy::canReallocateInplace>());
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
			pvCreate();
		}

		template<typename RelocateFunc>
		void Reset(size_t capacity, size_t count, RelocateFunc relocateFunc)
		{
			MOMO_ASSERT(count <= capacity);
			pvCheckCapacity(capacity);
			if (capacity > internalCapacity)
			{
				Item* items = MemManagerProxy::template Allocate<Item>(GetMemManager(),
					capacity * sizeof(Item));
				try
				{
					relocateFunc(items);
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
				pvReset(count, relocateFunc);
			}
		}

	private:
		static void pvCheckCapacity(size_t capacity)
		{
			if (capacity > SIZE_MAX / sizeof(Item))
				throw std::length_error("momo::Array length error");
		}

		template<bool hasInternalCapacity = (internalCapacity > 0)>
		internal::EnableIf<hasInternalCapacity> pvCreate() noexcept
		{
			mItems = &mInternalItems;
			mCount = 0;
		}

		template<bool hasInternalCapacity = (internalCapacity > 0)>
		internal::EnableIf<!hasInternalCapacity> pvCreate() noexcept
		{
			mItems = nullptr;
			mCount = 0;
			mCapacity = 0;
		}

		template<bool hasInternalCapacity = (internalCapacity > 0)>
		internal::EnableIf<hasInternalCapacity> pvCreateMove(Data&& data) noexcept
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
			data.pvCreate();
		}

		template<bool hasInternalCapacity = (internalCapacity > 0)>
		internal::EnableIf<!hasInternalCapacity> pvCreateMove(Data&& data) noexcept
		{
			mItems = data.mItems;
			mCount = data.mCount;
			mCapacity = data.mCapacity;
			data.pvCreate();
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

		bool pvIsInternal() const noexcept
		{
			return mItems == &mInternalItems;
		}

		template<bool canReallocateInplace>
		bool pvSetCapacity(size_t capacity, std::true_type /*canReallocate*/,
			internal::BoolConstant<canReallocateInplace>)
		{
			mItems = MemManagerProxy::template Reallocate<Item>(GetMemManager(),
				mItems, mCapacity * sizeof(Item), capacity * sizeof(Item));
			mCapacity = capacity;
			return true;
		}

		bool pvSetCapacity(size_t capacity, std::false_type /*canReallocate*/,
			std::true_type /*canReallocateInplace*/) noexcept
		{
			bool reallocDone = MemManagerProxy::ReallocateInplace(GetMemManager(),
				mItems, mCapacity * sizeof(Item), capacity * sizeof(Item));
			if (!reallocDone)
				return false;
			mCapacity = capacity;
			return true;
		}

		bool pvSetCapacity(size_t /*capacity*/, std::false_type /*canReallocate*/,
			std::false_type /*canReallocateInplace*/) noexcept
		{
			return false;
		}

		template<typename RelocateFunc,
			bool hasInternalCapacity = (internalCapacity > 0)>
		internal::EnableIf<hasInternalCapacity> pvReset(size_t count, RelocateFunc relocateFunc)
		{
			MOMO_STATIC_ASSERT(ItemTraits::isNothrowRelocatable);
			internal::ArrayBuffer<ItemTraits, internalCapacity> internalData;
			relocateFunc(&internalData);
			pvDeallocate();
			mItems = &mInternalItems;
			ItemTraits::Relocate(GetMemManager(), &internalData, mItems, count);
			mCount = count;
		}

		template<typename RelocateFunc,
			bool hasInternalCapacity = (internalCapacity > 0)>
		internal::EnableIf<!hasInternalCapacity> pvReset(size_t count,
			RelocateFunc /*relocateFunc*/) noexcept
		{
			(void)count;
			MOMO_ASSERT(count == 0);
			pvDeallocate();
			pvCreate();
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

public:
	typedef typename IteratorSelector::ConstIterator ConstIterator;
	typedef typename IteratorSelector::Iterator Iterator;

public:
	Array() noexcept(noexcept(MemManager()))
		: Array(MemManager())
	{
	}

	explicit Array(MemManager&& memManager) noexcept
		: mData(std::move(memManager))
	{
	}

	explicit Array(size_t count, MemManager&& memManager = MemManager())
		: mData(count, std::move(memManager))
	{
		for (size_t i = 0; i < count; ++i)
			AddBackNogrowVar();
	}

	explicit Array(size_t count, const Item& item, MemManager&& memManager = MemManager())
		: mData(count, std::move(memManager))
	{
		for (size_t i = 0; i < count; ++i)
			AddBackNogrow(item);
	}

	template<typename ArgIterator,
		typename = typename std::iterator_traits<ArgIterator>::iterator_category>
	explicit Array(ArgIterator begin, ArgIterator end, MemManager&& memManager = MemManager())
		: mData(internal::IsForwardIterator<ArgIterator>::value ? std::distance(begin, end) : 0,
			std::move(memManager))
	{
		pvFill(begin, end);
	}

	Array(std::initializer_list<Item> items, MemManager&& memManager = MemManager())
		: Array(items.begin(), items.end(), std::move(memManager))
	{
	}

	Array(Array&& array) noexcept
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

	template<typename MultiItemCreator>
	static Array CreateCrt(size_t count, const MultiItemCreator& multiItemCreator,
		MemManager&& memManager = MemManager())
	{
		Array array = CreateCap(count, std::move(memManager));
		for (size_t i = 0; i < count; ++i)
			array.AddBackNogrowCrt(multiItemCreator);
		return array;
	}

	~Array() noexcept
	{
	}

	Array& operator=(Array&& array) noexcept
	{
		if (this != &array)
			pvAssign(std::move(array));
		return *this;
	}

	Array& operator=(const Array& array)
	{
		if (this != &array)
			pvAssign(Array(array));
		return *this;
	}

	void Swap(Array& array) noexcept
	{
		if (this != &array)
		{
			Array tempArray(std::move(array));
			array.pvAssign(std::move(*this));
			pvAssign(std::move(tempArray));
		}
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
	MOMO_FRIENDS_BEGIN_END(const Array&, ConstIterator)
	MOMO_FRIENDS_BEGIN_END(Array&, Iterator)

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
			auto relocateFunc = [this, initCount, newCount, &multiItemCreator] (Item* newItems)
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
			mData.Reset(newCapacity, newCount, relocateFunc);
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
			InsertCrt(index, typename ItemTraits::template Creator<Item&&>(
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
			MemManager& memManager = GetMemManager();
			ItemHandler itemHandler(memManager,
				typename ItemTraits::template Creator<const Item&>(memManager, item));
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
		MOMO_ASSERT(begin == end || !pvIsInside(*begin));	//?
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

	template<typename ItemArg,
		typename Predicate = internal::TransparentEqualer>
	bool Contains(const ItemArg& itemArg, const Predicate& pred = Predicate()) const
	{
		const Item* begin = GetItems();
		const Item* end = begin + GetCount();
		return std::find_if(begin, end,
			[&itemArg, &pred] (const Item& item) { return pred(itemArg, item); }) != end;
	}

private:
	explicit Array(Data&& data) noexcept
		: mData(std::move(data))
	{
	}

	void pvAssign(Array&& array) noexcept
	{
		MOMO_ASSERT(this != &array);
		mData.~Data();	//?
		new(&mData) Data(std::move(array.mData));
	}

	template<typename ArgIterator>
	internal::EnableIf<internal::IsForwardIterator<ArgIterator>::value> pvFill(
		ArgIterator begin, ArgIterator end)
	{
		typedef typename ItemTraits::template Creator<
			typename std::iterator_traits<ArgIterator>::reference> IterCreator;
		MemManager& memManager = GetMemManager();
		for (ArgIterator iter = begin; iter != end; ++iter)
			AddBackNogrowCrt(IterCreator(memManager, *iter));
	}

	template<typename ArgIterator>
	internal::EnableIf<!internal::IsForwardIterator<ArgIterator>::value> pvFill(
		ArgIterator begin, ArgIterator end)
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

	template<typename Item>
	Item& pvGetItem(Item* items, size_t index) const
	{
		MOMO_CHECK(index < GetCount());
		return items[index];
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
		auto relocateFunc = [this, initCount, &itemCreator] (Item* newItems)
		{
			ItemTraits::RelocateCreate(GetMemManager(), GetItems(), newItems, initCount,
				std::forward<ItemCreator>(itemCreator), newItems + initCount);
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
		typename ItemTraits::template Creator<Item&&>(GetMemManager(),
			std::move(itemIndex == SIZE_MAX ? item : items[itemIndex]))(items + initCount);
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

	void pvRemoveBack(size_t count) noexcept
	{
		size_t initCount = GetCount();
		ItemTraits::Destroy(GetMemManager(), GetItems() + initCount - count, count);
		mData.SetCount(initCount - count);
	}

	size_t pvIndexOf(const Item& item) const noexcept
	{
		const Item* pitem = std::addressof(item);
		const Item* items = GetItems();
		std::less<const Item*> less;
		return (!less(pitem, items) && less(pitem, items + GetCount())) ? pitem - items : SIZE_MAX;
	}

	template<typename ItemArg>
	bool pvIsInside(const ItemArg& itemArg) const noexcept
	{
		const void* pitem = std::addressof(itemArg);
		const Item* items = GetItems();
		std::less<const void*> less;
		return !less(pitem, items) && less(pitem, items + GetCount());
	}

private:
	Data mData;
};

#ifdef MOMO_HAS_DEDUCTION_GUIDES
template<typename ArgIterator,
	typename MemManager = MemManagerDefault>
Array(ArgIterator, ArgIterator, MemManager = MemManager())
	-> Array<typename std::iterator_traits<ArgIterator>::value_type, MemManager>;
#endif

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

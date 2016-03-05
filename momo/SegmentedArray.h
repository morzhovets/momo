/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/SegmentedArray.h

  namespace momo:
    struct SegmentedArrayItemTraits
    enum class SegmentedArrayItemCountFunc
    struct SegmentedArraySettings
    class SegmentedArray

\**********************************************************/

#pragma once

#include "Array.h"

namespace momo
{

template<typename TItem>
struct SegmentedArrayItemTraits
{
	typedef TItem Item;

	typedef internal::ObjectManager<Item> ItemManager;

	static const size_t alignment = ItemManager::alignment;

	template<typename... ItemArgs>
	using Creator = typename ItemManager::template Creator<ItemArgs...>;

	static void Destroy(Item* items, size_t count) MOMO_NOEXCEPT
	{
		ItemManager::Destroy(items, count);
	}

	template<typename Arg>
	static void Assign(Arg&& arg, Item& item)
	{
		item = std::forward<Arg>(arg);
	}
};

enum class SegmentedArrayItemCountFunc
{
	sqrt = 0,
	cnst = 1,
};

namespace internal
{
	template<SegmentedArrayItemCountFunc tItemCountFunc>
	struct SegmentedArrayLogFirstItemCounter;

	template<>
	struct SegmentedArrayLogFirstItemCounter<SegmentedArrayItemCountFunc::sqrt>
	{
		static const size_t defaultLogFirstItemCount = 3;
	};

	template<>
	struct SegmentedArrayLogFirstItemCounter<SegmentedArrayItemCountFunc::cnst>
	{
		static const size_t defaultLogFirstItemCount = 5;
	};
}

template<SegmentedArrayItemCountFunc tItemCountFunc = SegmentedArrayItemCountFunc::sqrt,
	size_t tLogFirstItemCount =
		internal::SegmentedArrayLogFirstItemCounter<tItemCountFunc>::defaultLogFirstItemCount>
struct SegmentedArraySettings;

template<size_t tLogFirstItemCount>
struct SegmentedArraySettings<SegmentedArrayItemCountFunc::sqrt, tLogFirstItemCount>
{
	static const CheckMode checkMode = CheckMode::bydefault;

	static const SegmentedArrayItemCountFunc itemCountFunc = SegmentedArrayItemCountFunc::sqrt;
	static const size_t logFirstItemCount = tLogFirstItemCount;

	typedef ArraySettings<> SegmentsSettings;

	static void GetSegItemIndices(size_t index, size_t& segIndex, size_t& itemIndex) MOMO_NOEXCEPT
	{
		size_t index1 = (index >> logFirstItemCount) + 1;
		size_t index2 = index & (((size_t)1 << logFirstItemCount) - 1);
		size_t logItemCount = _IndexToLogItemCount(index1);
		size_t itemIndex1 = index1 & (((size_t)1 << logItemCount) - 1);
		size_t itemIndex2 = index2;
		segIndex = (index1 >> logItemCount) + ((size_t)1 << logItemCount) - 2;
		itemIndex = (itemIndex1 << logFirstItemCount) + itemIndex2;
	}

	static size_t GetIndex(size_t segIndex, size_t itemIndex) MOMO_NOEXCEPT
	{
		size_t itemIndex1 = itemIndex >> logFirstItemCount;
		size_t itemIndex2 = itemIndex & (((size_t)1 << logFirstItemCount) - 1);
		size_t logItemCount = _SegIndexToLogItemCount(segIndex);
		size_t index1 = ((segIndex + 2 - ((size_t)1 << logItemCount)) << logItemCount) + itemIndex1;
		size_t index2 = itemIndex2;
		size_t index = ((index1 - 1) << logFirstItemCount) + index2;
		return index;
	}

	static size_t GetItemCount(size_t segIndex) MOMO_NOEXCEPT
	{
		size_t logItemCount = _SegIndexToLogItemCount(segIndex);
		return (size_t)1 << (logItemCount + logFirstItemCount);
	}

private:
	static size_t _IndexToLogItemCount(size_t index1) MOMO_NOEXCEPT
	{
		return (internal::UIntMath<size_t>::Log2(index1) + 1) / 2;
	}

	static size_t _SegIndexToLogItemCount(size_t segIndex) MOMO_NOEXCEPT
	{
		return internal::UIntMath<size_t>::Log2((segIndex * 2 + 4) / 3);
	}
};

template<size_t tLogFirstItemCount>
struct SegmentedArraySettings<SegmentedArrayItemCountFunc::cnst, tLogFirstItemCount>
{
	static const CheckMode checkMode = CheckMode::bydefault;

	static const SegmentedArrayItemCountFunc itemCountFunc = SegmentedArrayItemCountFunc::cnst;
	static const size_t logFirstItemCount = tLogFirstItemCount;

	typedef ArraySettings<> SegmentsSettings;

	static void GetSegItemIndices(size_t index, size_t& segIndex, size_t& itemIndex) MOMO_NOEXCEPT
	{
		segIndex = index >> logFirstItemCount;
		itemIndex = index & (((size_t)1 << logFirstItemCount) - 1);
	}

	static size_t GetIndex(size_t segIndex, size_t itemIndex) MOMO_NOEXCEPT
	{
		return (segIndex << logFirstItemCount) + itemIndex;
	}

	static size_t GetItemCount(size_t /*segIndex*/) MOMO_NOEXCEPT
	{
		return (size_t)1 << logFirstItemCount;
	}
};

template<typename TItem,
	typename TMemManager = MemManagerDefault,
	typename TItemTraits = SegmentedArrayItemTraits<TItem>,
	typename TSettings = SegmentedArraySettings<>>
class SegmentedArray
{
public:
	typedef TItem Item;
	typedef TMemManager MemManager;
	typedef TItemTraits ItemTraits;
	typedef TSettings Settings;

	typedef internal::ArrayIterator<SegmentedArray, Item> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

private:
	struct SegmentsSettings : public Settings::SegmentsSettings
	{
		static const CheckMode checkMode = CheckMode::assertion;
	};

	typedef Array<Item*, MemManager, ArrayItemTraits<Item*>, SegmentsSettings> Segments;

	typedef internal::ArrayItemHandler<ItemTraits> ItemHandler;
	typedef internal::ArrayShifter<SegmentedArray> ArrayShifter;

public:
	SegmentedArray()
		: SegmentedArray(MemManager())
	{
	}

	explicit SegmentedArray(MemManager&& memManager) MOMO_NOEXCEPT
		: mSegments(std::move(memManager)),
		mCount(0)
	{
	}

	explicit SegmentedArray(size_t count, MemManager&& memManager = MemManager())
		: SegmentedArray(std::move(memManager))
	{
		_IncCount(count, typename ItemTraits::template Creator<>());
	}

	SegmentedArray(size_t count, const Item& item, MemManager&& memManager = MemManager())
		: SegmentedArray(std::move(memManager))
	{
		_IncCount(count, typename ItemTraits::template Creator<const Item&>(item));
	}

	template<typename Iterator>
	SegmentedArray(Iterator begin, Iterator end, MemManager&& memManager = MemManager())
		: SegmentedArray(std::move(memManager))
	{
		try
		{
			typedef typename ItemTraits::template Creator<
				typename std::iterator_traits<Iterator>::reference> IterCreator;
			for (Iterator iter = begin; iter != end; ++iter)
				AddBackCrt(IterCreator(*iter));
		}
		catch (...)
		{
			_DecCount(0);
			_DecCapacity(0);
			throw;
		}
	}

	SegmentedArray(std::initializer_list<Item> items, MemManager&& memManager = MemManager())
		: SegmentedArray(items.begin(), items.end(), std::move(memManager))
	{
	}

	SegmentedArray(SegmentedArray&& array) MOMO_NOEXCEPT
		: mSegments(std::move(array.mSegments)),
		mCount(array.mCount)
	{
		array.mCount = 0;
	}

	SegmentedArray(const SegmentedArray& array, bool shrink = true)
		: SegmentedArray(MemManager(array.GetMemManager()))
	{
		_IncCapacity(shrink ? array.GetCount() : array.GetCapacity());
		try
		{
			for (const Item& item : array)
				AddBackNogrow(item);
		}
		catch (...)
		{
			_DecCount(0);
			_DecCapacity(0);
			throw;
		}
	}

	static SegmentedArray CreateCap(size_t capacity, MemManager&& memManager = MemManager())
	{
		SegmentedArray array(std::move(memManager));
		array._IncCapacity(capacity);
		return array;
	}

	template<typename ItemCreator>
	static SegmentedArray CreateCrt(size_t count, const ItemCreator& itemCreator,
		MemManager&& memManager = MemManager())
	{
		SegmentedArray array = CreateCap(count, std::move(memManager));
		array._IncCount(count, itemCreator);
		return array;
	}

	~SegmentedArray() MOMO_NOEXCEPT
	{
		_DecCount(0);
		_DecCapacity(0);
	}

	SegmentedArray& operator=(SegmentedArray&& array) MOMO_NOEXCEPT
	{
		SegmentedArray(std::move(array)).Swap(*this);
		return *this;
	}

	SegmentedArray& operator=(const SegmentedArray& array)
	{
		if (this != &array)
			SegmentedArray(array).Swap(*this);
		return *this;
	}

	void Swap(SegmentedArray& array) MOMO_NOEXCEPT
	{
		mSegments.Swap(array.mSegments);
		std::swap(mCount, array.mCount);
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
		return ConstIterator(this, mCount);
	}

	Iterator GetEnd() MOMO_NOEXCEPT
	{
		return Iterator(this, mCount);
	}

	MOMO_FRIEND_SWAP(SegmentedArray)
	MOMO_FRIENDS_BEGIN_END(const SegmentedArray&, ConstIterator)
	MOMO_FRIENDS_BEGIN_END(SegmentedArray&, Iterator)

	const MemManager& GetMemManager() const MOMO_NOEXCEPT
	{
		return mSegments.GetMemManager();
	}

	MemManager& GetMemManager() MOMO_NOEXCEPT
	{
		return mSegments.GetMemManager();
	}

	size_t GetCount() const MOMO_NOEXCEPT
	{
		return mCount;
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
		return mCount == 0;
	}

	void Clear(bool shrink = false) MOMO_NOEXCEPT
	{
		_DecCount(0);
		if (shrink)
		{
			_DecCapacity(0);
			mSegments.Clear(true);
		}
	}

	size_t GetCapacity() const MOMO_NOEXCEPT
	{
		return Settings::GetIndex(mSegments.GetCount(), 0);	//?
	}

	void Reserve(size_t capacity)
	{
		if (capacity > GetCapacity())
			_IncCapacity(capacity);
	}

	void Shrink() MOMO_NOEXCEPT
	{
		_DecCapacity(mCount);
		try
		{
			mSegments.Shrink();
		}
		catch (...)
		{
			// no throw!
		}
	}

	const Item& operator[](size_t index) const
	{
		return _GetItem(index);
	}

	Item& operator[](size_t index)
	{
		return _GetItem(index);
	}

	const Item& GetBackItem() const
	{
		return _GetItem(mCount - 1);
	}

	Item& GetBackItem()
	{
		return _GetItem(mCount - 1);
	}

	template<typename ItemCreator>
	void AddBackNogrowCrt(const ItemCreator& itemCreator)
	{
		MOMO_CHECK(mCount < GetCapacity());
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
		size_t initCapacity = GetCapacity();
		if (mCount < initCapacity)
		{
			_AddBackNogrow(itemCreator);
		}
		else
		{
			_IncCapacity(initCapacity + 1);
			try
			{
				itemCreator(mSegments.GetBackItem());
				++mCount;
			}
			catch (...)
			{
				_DecCapacity(initCapacity);
				throw;
			}
		}
	}

	template<typename... ItemArgs>
	void AddBackVar(ItemArgs&&... itemArgs)
	{
		AddBackCrt(typename ItemTraits::template Creator<ItemArgs...>(
			std::forward<ItemArgs>(itemArgs)...));
	}

	void AddBack(Item&& item)
	{
		AddBackVar(std::move(item));
	}

	void AddBack(const Item& item)
	{
		AddBackVar(item);
	}

	// basic exception safety
	template<typename ItemCreator>
	void InsertCrt(size_t index, const ItemCreator& itemCreator)
	{
		ItemHandler itemHandler(itemCreator);
		std::move_iterator<Item*> begin(&itemHandler);
		Insert(index, begin, begin + 1);
	}

	// basic exception safety
	template<typename... ItemArgs>
	void InsertVar(size_t index, ItemArgs&&... itemArgs)
	{
		InsertCrt(index, typename ItemTraits::template Creator<ItemArgs...>(
			std::forward<ItemArgs>(itemArgs)...));
	}

	// basic exception safety
	void Insert(size_t index, Item&& item)
	{
		InsertVar(index, std::move(item));
	}

	// basic exception safety
	void Insert(size_t index, const Item& item)
	{
		InsertVar(index, item);
	}

	// basic exception safety
	void Insert(size_t index, size_t count, const Item& item)
	{
		typename ItemTraits::template Creator<const Item&> itemCreator(item);
		ItemHandler itemHandler(itemCreator);
		Reserve(mCount + count);
		ArrayShifter::Insert(*this, index, count, *&itemHandler);
	}

	// basic exception safety
	template<typename Iterator>
	void Insert(size_t index, Iterator begin, Iterator end)
	{
		if (internal::IsForwardIterator<Iterator>::value)
			Reserve(mCount + std::distance(begin, end));
		ArrayShifter::Insert(*this, index, begin, end, internal::IsForwardIterator<Iterator>());
	}

	// basic exception safety
	void Insert(size_t index, std::initializer_list<Item> items)
	{
		Insert(index, items.begin(), items.end());
	}

	void RemoveBack(size_t count = 1)
	{
		MOMO_CHECK(count <= mCount);
		_DecCount(mCount - count);
	}

	// !std::is_nothrow_move_assignable<Item>::value -> basic exception safety
	void Remove(size_t index, size_t count)
	{
		ArrayShifter::Remove(*this, index, count);
	}

private:
	Item* _GetSegMemory(size_t segIndex)
	{
		size_t itemCount = Settings::GetItemCount(segIndex);
		if (itemCount > SIZE_MAX / sizeof(Item))
			throw std::length_error("momo::SegmentedArray length error");
		return GetMemManager().template Allocate<Item>(itemCount * sizeof(Item));
	}

	void _FreeSegMemory(size_t segIndex, Item* segMemory) MOMO_NOEXCEPT
	{
		size_t itemCount = Settings::GetItemCount(segIndex);
		GetMemManager().Deallocate(segMemory, itemCount * sizeof(Item));
	}

	Item& _GetItem(size_t index) const
	{
		MOMO_CHECK(index < mCount);
		size_t segIndex, itemIndex;
		Settings::GetSegItemIndices(index, segIndex, itemIndex);
		return mSegments[segIndex][itemIndex];
	}

	template<typename ItemCreator>
	void _AddBackNogrow(const ItemCreator& itemCreator)
	{
		size_t segIndex, itemIndex;
		Settings::GetSegItemIndices(mCount, segIndex, itemIndex);
		itemCreator(mSegments[segIndex] + itemIndex);
		++mCount;
	}

	template<typename ItemCreator>
	void _SetCount(size_t count, const ItemCreator& itemCreator)
	{
		if (count < mCount)
			_DecCount(count);
		else if (count > mCount)
			_IncCount(count, itemCreator);
	}

	template<typename ItemCreator>
	void _IncCount(size_t count, const ItemCreator& itemCreator)
	{
		MOMO_ASSERT(count >= mCount);
		size_t initCapacity = GetCapacity();
		size_t initCount = mCount;
		Reserve(count);
		try
		{
			size_t segIndex, itemIndex;
			Settings::GetSegItemIndices(mCount, segIndex, itemIndex);
			while (mCount < count)
			{
				Item* segment = mSegments[segIndex];
				size_t itemCount = Settings::GetItemCount(segIndex);
				for (; itemIndex < itemCount && mCount < count; ++itemIndex, ++mCount)
					itemCreator(segment + itemIndex);
				if (itemIndex == itemCount)
				{
					++segIndex;
					itemIndex = 0;
				}
			}
		}
		catch (...)
		{
			_DecCount(initCount);
			_DecCapacity(initCapacity);
			throw;
		}
	}

	void _DecCount(size_t count) MOMO_NOEXCEPT
	{
		MOMO_ASSERT(count <= mCount);
		size_t segIndex, itemIndex;
		Settings::GetSegItemIndices(mCount, segIndex, itemIndex);
		while (mCount > count)
		{
			if (itemIndex == 0)
			{
				--segIndex;
				itemIndex = Settings::GetItemCount(segIndex);
			}
			size_t delCount = std::minmax(itemIndex, mCount - count).first;
			ItemTraits::Destroy(mSegments[segIndex] + itemIndex - delCount, delCount);
			itemIndex -= delCount;
			mCount -= delCount;
		}
	}

	void _IncCapacity(size_t capacity)
	{
		size_t initCapacity = GetCapacity();
		MOMO_ASSERT(capacity >= initCapacity);
		size_t segIndex, itemIndex;
		Settings::GetSegItemIndices(capacity, segIndex, itemIndex);
		if (itemIndex > 0)
			++segIndex;
		try
		{
			for (size_t segCount = mSegments.GetCount(); segCount < segIndex; ++segCount)
			{
				mSegments.Reserve(segCount + 1);
				Item* segMemory = _GetSegMemory(segCount);
				mSegments.AddBackNogrow(segMemory);
			}
		}
		catch (...)
		{
			_DecCapacity(initCapacity);
			throw;
		}
	}

	void _DecCapacity(size_t capacity) MOMO_NOEXCEPT
	{
		MOMO_ASSERT(capacity <= GetCapacity());
		size_t segIndex, itemIndex;
		Settings::GetSegItemIndices(capacity, segIndex, itemIndex);
		if (itemIndex > 0)
			++segIndex;
		size_t segCount = mSegments.GetCount();
		for (size_t i = segIndex; i < segCount; ++i)
			_FreeSegMemory(i, mSegments[i]);
		mSegments.RemoveBack(segCount - segIndex);
	}

private:
	Segments mSegments;
	size_t mCount;
};

} // namespace momo

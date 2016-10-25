/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/ArrayUtility.h

\**********************************************************/

#pragma once

#include "ObjectManager.h"

#define MOMO_MORE_ARRAY_ITERATOR_OPERATORS(Iterator) \
	Iterator& operator++() \
	{ \
		return *this += 1; \
	} \
	Iterator operator++(int) \
	{ \
		Iterator tempIter = *this; \
		++*this; \
		return tempIter; \
	} \
	Iterator& operator--() \
	{ \
		return *this -= 1; \
	} \
	Iterator operator--(int) \
	{ \
		Iterator tempIter = *this; \
		--*this; \
		return tempIter; \
	} \
	Iterator operator+(ptrdiff_t diff) const \
	{ \
		return Iterator(*this) += diff; \
	} \
	friend Iterator operator+(ptrdiff_t diff, Iterator iter) \
	{ \
		return iter + diff; \
	} \
	Iterator& operator-=(ptrdiff_t diff) \
	{ \
		return *this += (-diff); \
	} \
	Iterator operator-(ptrdiff_t diff) const \
	{ \
		return *this + (-diff); \
	} \
	Item* operator->() const \
	{ \
		return std::addressof(**this); \
	} \
	Item& operator[](ptrdiff_t diff) const \
	{ \
		return *(*this + diff); \
	} \
	bool operator!=(ConstIterator iter) const MOMO_NOEXCEPT \
	{ \
		return !(*this == iter); \
	} \
	bool operator>(ConstIterator iter) const \
	{ \
		return iter < *this; \
	} \
	bool operator<=(ConstIterator iter) const \
	{ \
		return !(iter < *this); \
	} \
	bool operator>=(ConstIterator iter) const \
	{ \
		return iter <= *this; \
	}

namespace momo
{

namespace internal
{
	template<typename TArray, typename TItem>
	class ArrayIndexIterator
	{
	public:
		typedef TArray Array;
		typedef TItem Item;
		typedef typename Array::Settings Settings;

		typedef Item& Reference;
		typedef Item* Pointer;

		typedef ArrayIndexIterator<const Array, const Item> ConstIterator;

	public:
		ArrayIndexIterator() MOMO_NOEXCEPT
			: mArray(nullptr),
			mIndex(0)
		{
		}

		ArrayIndexIterator(Array* array, size_t index) MOMO_NOEXCEPT
			: mArray(array),
			mIndex(index)
		{
		}

		operator ConstIterator() const MOMO_NOEXCEPT
		{
			return ConstIterator(mArray, mIndex);
		}

		ArrayIndexIterator& operator+=(ptrdiff_t diff)
		{
			MOMO_CHECK(mArray != nullptr);
			size_t newIndex = mIndex + diff;
			MOMO_CHECK(newIndex <= mArray->GetCount());
			mIndex = newIndex;
			return *this;
		}

		ptrdiff_t operator-(ConstIterator iter) const
		{
			MOMO_CHECK(mArray == iter.GetArray());
			return mIndex - iter.GetIndex();
		}

		Item& operator*() const
		{
			MOMO_CHECK(mArray != nullptr);
			return (*mArray)[mIndex];
		}

		bool operator==(ConstIterator iter) const MOMO_NOEXCEPT
		{
			return mArray == iter.GetArray() && mIndex == iter.GetIndex();
		}

		bool operator<(ConstIterator iter) const
		{
			MOMO_CHECK(mArray == iter.GetArray());
			return mIndex < iter.GetIndex();
		}

		MOMO_MORE_ARRAY_ITERATOR_OPERATORS(ArrayIndexIterator)

		Array* GetArray() const MOMO_NOEXCEPT
		{
			return mArray;
		}

		size_t GetIndex() const MOMO_NOEXCEPT
		{
			return mIndex;
		}

	private:
		Array* mArray;
		size_t mIndex;
	};

	template<typename TItemTraits, size_t tCount>
	class ArrayBuffer
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef typename ItemTraits::Item Item;

		static const size_t count = tCount;

	public:
		const Item* operator&() const MOMO_NOEXCEPT
		{
			return &*mItems;
		}

		Item* operator&() MOMO_NOEXCEPT
		{
			return &*mItems;
		}

	private:
		ObjectBuffer<Item, ItemTraits::alignment> mItems[count];
	};

	template<typename TItemTraits>
	class ArrayBuffer<TItemTraits, 0>
	{
	public:
		typedef TItemTraits ItemTraits;
		//typedef typename ItemTraits::Item Item;

		static const size_t count = 0;

	public:
		const void* operator&() const MOMO_NOEXCEPT
		{
			return this;
		}

		void* operator&() MOMO_NOEXCEPT
		{
			return this;
		}
	};

	template<typename TItemTraits>
	class ArrayItemHandler
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

	public:
		template<typename ItemCreator>
		explicit ArrayItemHandler(MemManager& memManager, const ItemCreator& itemCreator)
			: mMemManager(memManager)
		{
			itemCreator(&mItemBuffer);
		}

		ArrayItemHandler(const ArrayItemHandler&) = delete;

		~ArrayItemHandler() MOMO_NOEXCEPT
		{
			ItemTraits::Destroy(mMemManager, &mItemBuffer, 1);
		}

		ArrayItemHandler& operator=(const ArrayItemHandler&) = delete;

		Item* operator&() MOMO_NOEXCEPT
		{
			return &mItemBuffer;
		}

	private:
		ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
		MemManager& mMemManager;
	};

	template<typename TArray>
	class ArrayShifter
	{
	public:
		typedef TArray Array;
		typedef typename Array::Item Item;
		typedef typename Array::MemManager MemManager;
		typedef typename Array::ItemTraits ItemTraits;
		typedef typename Array::Settings Settings;

	public:
		static void Insert(Array& array, size_t index, size_t count, const Item& item)
		{
			size_t initCount = array.GetCount();
			MOMO_CHECK(index <= initCount);
			MOMO_ASSERT(array.GetCapacity() >= initCount + count);
			MemManager& memManager = array.GetMemManager();
			if (index + count < initCount)
			{
				for (size_t i = initCount - count; i < initCount; ++i)
					array.AddBackNogrow(std::move(array[i]));
				for (size_t i = initCount - count; i > index; --i)
					ItemTraits::Assign(memManager, std::move(array[i - 1]), array[i + count - 1]);
				for (size_t i = index; i < index + count; ++i)
					ItemTraits::Assign(memManager, item, array[i]);
			}
			else
			{
				for (size_t i = initCount; i < index + count; ++i)
					array.AddBackNogrow(item);
				for (size_t i = index; i < initCount; ++i)
				{
					Item& arrayItem = array[i];
					array.AddBackNogrow(std::move(arrayItem));
					ItemTraits::Assign(memManager, item, arrayItem);
				}
			}
		}

		template<typename ArgIterator>
		static void Insert(Array& array, size_t index, ArgIterator begin, ArgIterator end,
			typename std::enable_if<IsForwardIterator<ArgIterator>::value, int>::type = 0)
		{
			size_t initCount = array.GetCount();
			MOMO_CHECK(index <= initCount);
			size_t count = std::distance(begin, end);
			MOMO_ASSERT(array.GetCapacity() >= initCount + count);
			MemManager& memManager = array.GetMemManager();
			if (index + count < initCount)
			{
				for (size_t i = initCount - count; i < initCount; ++i)
					array.AddBackNogrow(std::move(array[i]));
				for (size_t i = initCount - count; i > index; --i)
					ItemTraits::Assign(memManager, std::move(array[i - 1]), array[i + count - 1]);
				ArgIterator iter = begin;
				for (size_t i = index; i < index + count; ++i, ++iter)
					ItemTraits::Assign(memManager, *iter, array[i]);
			}
			else
			{
				typedef typename ItemTraits::template Creator<
					typename std::iterator_traits<ArgIterator>::reference> IterCreator;
				ArgIterator iter = std::next(begin, initCount - index);
				for (size_t i = initCount; i < index + count; ++i, ++iter)
					array.AddBackNogrowCrt(IterCreator(memManager, *iter));
				iter = begin;
				for (size_t i = index; i < initCount; ++i, ++iter)
				{
					Item& arrayItem = array[i];
					array.AddBackNogrow(std::move(arrayItem));
					ItemTraits::Assign(memManager, *iter, arrayItem);
				}
			}
		}

		template<typename ArgIterator>
		static void Insert(Array& array, size_t index, ArgIterator begin, ArgIterator end,
			typename std::enable_if<!IsForwardIterator<ArgIterator>::value, int>::type = 0)
		{
			typedef typename ItemTraits::template Creator<
				typename std::iterator_traits<ArgIterator>::reference> IterCreator;
			MemManager& memManager = array.GetMemManager();
			size_t count = 0;
			for (ArgIterator iter = begin; iter != end; ++iter, ++count)
				array.InsertCrt(index + count, IterCreator(memManager, *iter));
		}

		static void Remove(Array& array, size_t index, size_t count)
		{
			size_t initCount = array.GetCount();
			MOMO_CHECK(index + count <= initCount);
			MemManager& memManager = array.GetMemManager();
			for (size_t i = index + count; i < initCount; ++i)
				ItemTraits::Assign(memManager, std::move(array[i]), array[i - count]);
			array.RemoveBack(count);
		}
	};
}

} // namespace momo

namespace std
{
	template<typename A, typename I>
	struct iterator_traits<momo::internal::ArrayIndexIterator<A, I>>
		: public iterator_traits<I*>
	{
	};
} // namespace std

/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/ArrayUtility.h

\**********************************************************/

#pragma once

#include "ObjectManager.h"

namespace momo
{

namespace internal
{
	template<typename TArray, typename TItem>
	class ArrayIterator
	{
	public:
		typedef TArray Array;
		typedef TItem Item;
		typedef typename Array::Settings Settings;

		typedef Item& Reference;
		typedef Item* Pointer;

		typedef ArrayIterator<const Array, const Item> ConstIterator;

	public:
		ArrayIterator() MOMO_NOEXCEPT
			: mArray(nullptr),
			mIndex(0)
		{
		}

		ArrayIterator(Array* array, size_t index) MOMO_NOEXCEPT
			: mArray(array),
			mIndex(index)
		{
		}

		operator ConstIterator() const MOMO_NOEXCEPT
		{
			return ConstIterator(mArray, mIndex);
		}

		ArrayIterator& operator++()
		{
			return *this += 1;
		}

		ArrayIterator operator++(int)
		{
			ArrayIterator tempIter = *this;
			++*this;
			return tempIter;
		}

		ArrayIterator& operator--()
		{
			return *this -= 1;
		}

		ArrayIterator operator--(int)
		{
			ArrayIterator tempIter = *this;
			--*this;
			return tempIter;
		}

		ArrayIterator& operator+=(ptrdiff_t diff)
		{
			MOMO_CHECK(mArray != nullptr);
			size_t newIndex = mIndex + diff;
			MOMO_CHECK(newIndex <= mArray->GetCount());
			mIndex = newIndex;
			return *this;
		}

		ArrayIterator operator+(ptrdiff_t diff) const
		{
			return ArrayIterator(*this) += diff;
		}

		friend ArrayIterator operator+(ptrdiff_t diff, ArrayIterator iter)
		{
			return iter + diff;
		}

		ArrayIterator& operator-=(ptrdiff_t diff)
		{
			return *this += (-diff);
		}

		ArrayIterator operator-(ptrdiff_t diff) const
		{
			return *this + (-diff);
		}

		ptrdiff_t operator-(ConstIterator iter) const
		{
			MOMO_CHECK(mArray == iter.GetArray());
			return mIndex - iter.GetIndex();
		}

		Item* operator->() const
		{
			return std::addressof(**this);
		}

		Item& operator*() const
		{
			MOMO_CHECK(mArray != nullptr);
			return (*mArray)[mIndex];
		}

		Item& operator[](ptrdiff_t diff) const
		{
			return *(*this + diff);
		}

		bool operator==(ConstIterator iter) const MOMO_NOEXCEPT
		{
			return mArray == iter.GetArray() && mIndex == iter.GetIndex();
		}

		bool operator!=(ConstIterator iter) const MOMO_NOEXCEPT
		{
			return !(*this == iter);
		}

		bool operator<(ConstIterator iter) const
		{
			MOMO_CHECK(mArray == iter.GetArray());
			return mIndex < iter.GetIndex();
		}

		bool operator>(ConstIterator iter) const
		{
			return iter < *this;
		}

		bool operator<=(ConstIterator iter) const
		{
			return !(iter < *this);
		}

		bool operator>=(ConstIterator iter) const
		{
			return iter <= *this;
		}

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

	public:
		template<typename ItemCreator>
		explicit ArrayItemHandler(const ItemCreator& itemCreator)
		{
			itemCreator(&mItemBuffer);
		}

		ArrayItemHandler(const ArrayItemHandler&) = delete;

		~ArrayItemHandler() MOMO_NOEXCEPT
		{
			ItemTraits::Destroy(&mItemBuffer, 1);
		}

		ArrayItemHandler& operator=(const ArrayItemHandler&) = delete;

		Item* operator&() MOMO_NOEXCEPT
		{
			return &mItemBuffer;
		}

	private:
		ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
	};

	template<typename TArray>
	struct ArrayShifter
	{
		typedef TArray Array;
		typedef typename Array::Item Item;
		typedef typename Array::MemManager MemManager;
		typedef typename Array::ItemTraits ItemTraits;
		typedef typename Array::Settings Settings;

		static void Insert(Array& array, size_t index, size_t count, const Item& item)
		{
			size_t initCount = array.GetCount();
			MOMO_CHECK(index <= initCount);
			MOMO_ASSERT(array.GetCapacity() >= initCount + count);
			if (index + count < initCount)
			{
				for (size_t i = initCount - count; i < initCount; ++i)
					array.AddBackNogrow(std::move(array[i]));
				for (size_t i = initCount - count; i > index; --i)
					ItemTraits::Assign(std::move(array[i - 1]), array[i + count - 1]);
				for (size_t i = index; i < index + count; ++i)
					ItemTraits::Assign(item, array[i]);
			}
			else
			{
				for (size_t i = initCount; i < index + count; ++i)
					array.AddBackNogrow(item);
				for (size_t i = index; i < initCount; ++i)
				{
					Item& arrayItem = array[i];
					array.AddBackNogrow(std::move(arrayItem));
					ItemTraits::Assign(item, arrayItem);
				}
			}
		}

		template<typename Iterator>
		static void Insert(Array& array, size_t index, Iterator begin, Iterator end,
			std::true_type /*isForwardIterator*/)
		{
			size_t initCount = array.GetCount();
			MOMO_CHECK(index <= initCount);
			size_t count = std::distance(begin, end);
			MOMO_ASSERT(array.GetCapacity() >= initCount + count);
			if (index + count < initCount)
			{
				for (size_t i = initCount - count; i < initCount; ++i)
					array.AddBackNogrow(std::move(array[i]));
				for (size_t i = initCount - count; i > index; --i)
					ItemTraits::Assign(std::move(array[i - 1]), array[i + count - 1]);
				Iterator iter = begin;
				for (size_t i = index; i < index + count; ++i, ++iter)
					ItemTraits::Assign(*iter, array[i]);
			}
			else
			{
				typedef typename ItemTraits::template Creator<
					typename std::iterator_traits<Iterator>::reference> IterCreator;
				Iterator iter = std::next(begin, initCount - index);
				for (size_t i = initCount; i < index + count; ++i, ++iter)
					array.AddBackNogrowCrt(IterCreator(*iter));
				iter = begin;
				for (size_t i = index; i < initCount; ++i, ++iter)
				{
					Item& arrayItem = array[i];
					array.AddBackNogrow(std::move(arrayItem));
					ItemTraits::Assign(*iter, arrayItem);
				}
			}
		}

		template<typename Iterator>
		static void Insert(Array& array, size_t index, Iterator begin, Iterator end,
			std::false_type /*isForwardIterator*/)
		{
			typedef typename ItemTraits::template Creator<
				typename std::iterator_traits<Iterator>::reference> IterCreator;
			size_t count = 0;
			for (Iterator iter = begin; iter != end; ++iter, ++count)
				array.InsertCrt(index + count, IterCreator(*iter));
		}

		static void Remove(Array& array, size_t index, size_t count)
		{
			size_t initCount = array.GetCount();
			MOMO_CHECK(index + count <= initCount);
			for (size_t i = index + count; i < initCount; ++i)
				ItemTraits::Assign(std::move(array[i]), array[i - count]);
			array.RemoveBack(count);
		}
	};
}

} // namespace momo

namespace std
{
	template<typename A, typename I>
	struct iterator_traits<momo::internal::ArrayIterator<A, I>>
		: public iterator_traits<I*>
	{
	};
} // namespace std

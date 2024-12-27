/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/ArrayUtility.h

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_ARRAY_UTILITY
#define MOMO_INCLUDE_GUARD_ARRAY_UTILITY

#include "ObjectManager.h"
#include "IteratorUtility.h"

namespace momo
{

namespace internal
{
	template<typename TQArray, typename TItem>
	class ArrayIndexIterator;

	template<typename QArray, typename QItem,
		typename = QItem*>
	struct ArrayIndexIteratorTraitsStd
		: public IteratorTraitsStd<ArrayIndexIterator<QArray, QItem>, std::random_access_iterator_tag>
	{
	};

	template<typename QArray, typename QItem>
	struct ArrayIndexIteratorTraitsStd<QArray, QItem, decltype(std::declval<QArray&>().GetItems())>
		: public std::iterator_traits<QItem*>
	{
	};

	template<typename TQArray, typename TQItem>
	class ArrayIndexIterator
	{
	protected:
		typedef TQItem QItem;
		typedef TQArray QArray;

		typedef typename QArray::Settings Settings;

	public:
		typedef QItem& Reference;
		typedef QItem* Pointer;

		typedef ArrayIndexIterator<const QArray, const QItem> ConstIterator;

	private:
		struct ConstIteratorProxy : public ConstIterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
		};

	public:
		explicit ArrayIndexIterator() noexcept
			: mArray(nullptr),
			mIndex(0)
		{
		}

		operator ConstIterator() const noexcept
		{
			return ConstIteratorProxy(mArray, mIndex);
		}

		ArrayIndexIterator& operator+=(ptrdiff_t diff)
		{
			size_t newIndex = static_cast<size_t>(static_cast<ptrdiff_t>(mIndex) + diff);
			MOMO_CHECK((mArray != nullptr) ? newIndex <= mArray->GetCount() : diff == 0);
			mIndex = newIndex;
			return *this;
		}

		friend ptrdiff_t operator-(ArrayIndexIterator iter1, ArrayIndexIterator iter2)
		{
			MOMO_CHECK(iter1.mArray == iter2.mArray);
			return static_cast<ptrdiff_t>(iter1.mIndex - iter2.mIndex);
		}

		Pointer operator->() const
		{
			MOMO_CHECK(mArray != nullptr);
			return pvGetPointer(std::is_base_of<std::iterator_traits<QItem*>,
				ArrayIndexIteratorTraitsStd<QArray, QItem>>());
		}

		friend bool operator==(ArrayIndexIterator iter1, ArrayIndexIterator iter2) noexcept
		{
			return iter1.mArray == iter2.mArray && iter1.mIndex == iter2.mIndex;
		}

#ifdef MOMO_HAS_THREE_WAY_COMPARISON
		friend auto operator<=>(ArrayIndexIterator iter1, ArrayIndexIterator iter2)
		{
			MOMO_CHECK(iter1.mArray == iter2.mArray);
			return iter1.mIndex <=> iter2.mIndex;
		}
#else
		friend bool operator<(ArrayIndexIterator iter1, ArrayIndexIterator iter2)
		{
			MOMO_CHECK(iter1.mArray == iter2.mArray);
			return iter1.mIndex < iter2.mIndex;
		}
#endif

		MOMO_MORE_ARRAY_ITERATOR_OPERATORS(ArrayIndexIterator)

	protected:
		explicit ArrayIndexIterator(QArray* array, size_t index) noexcept
			: mArray(array),
			mIndex(index)
		{
		}

		QArray* ptGetArray() const noexcept
		{
			return mArray;
		}

		size_t ptGetIndex() const noexcept
		{
			return mIndex;
		}

	private:
		Pointer pvGetPointer(std::true_type) const
		{
			return mArray->GetItems() + mIndex;
		}

		Pointer pvGetPointer(std::false_type) const
		{
			return std::addressof((*mArray)[mIndex]);
		}

	private:
		QArray* mArray;
		size_t mIndex;
	};

	template<typename TItemTraits>
	class ArrayItemHandler
	{
	protected:
		typedef TItemTraits ItemTraits;

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

	public:
		template<typename ItemCreator>
		explicit ArrayItemHandler(MemManager& memManager, ItemCreator&& itemCreator)
			: mMemManager(memManager)
		{
			std::forward<ItemCreator>(itemCreator)(mItemBuffer.GetPtr());
		}

		ArrayItemHandler(const ArrayItemHandler&) = delete;

		~ArrayItemHandler() noexcept
		{
			ItemTraits::Destroy(mMemManager, std::addressof(Get()), 1);
		}

		ArrayItemHandler& operator=(const ArrayItemHandler&) = delete;

		Item& Get() noexcept
		{
			return mItemBuffer.Get();
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
		static void InsertNogrow(Array& array, size_t index, size_t count, const Item& item)
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
		static EnableIf<IsForwardIterator17<ArgIterator>::value>
		InsertNogrow(Array& array, size_t index, ArgIterator begin, size_t count)
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
				ArgIterator iter = begin;
				for (size_t i = index; i < index + count; ++i, (void)++iter)
					ItemTraits::Assign(memManager, *iter, array[i]);
			}
			else
			{
				typedef typename ItemTraits::template Creator<decltype(*begin)> IterCreator;
				ArgIterator iter = UIntMath<>::Next(begin, initCount - index);
				for (size_t i = initCount; i < index + count; ++i, (void)++iter)
					array.AddBackNogrowCrt(IterCreator(memManager, *iter));
				iter = begin;
				for (size_t i = index; i < initCount; ++i, (void)++iter)
				{
					Item& arrayItem = array[i];
					array.AddBackNogrow(std::move(arrayItem));
					ItemTraits::Assign(memManager, *iter, arrayItem);
				}
			}
		}

		template<typename ArgIterator, typename ArgSentinel>
		static void Insert(Array& array, size_t index, ArgIterator begin, ArgSentinel end)
		{
			typedef typename ItemTraits::template Creator<decltype(*begin)> IterCreator;
			MemManager& memManager = array.GetMemManager();
			size_t count = 0;
			for (ArgIterator iter = std::move(begin); iter != end; (void)++iter, ++count)
				array.InsertCrt(index + count, IterCreator(memManager, *iter));
		}

		static void InsertNogrow(Array& array, size_t index, Item&& item)
		{
			InsertNogrow(array, index, std::make_move_iterator(std::addressof(item)), 1);
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

		template<typename ItemFilter>
		static size_t Remove(Array& array, const ItemFilter& itemFilter)
		{
			size_t initCount = array.GetCount();
			size_t newCount = 0;
			while (newCount < initCount && !itemFilter(static_cast<const Item&>(array[newCount])))
				++newCount;
			MemManager& memManager = array.GetMemManager();
			for (size_t i = newCount + 1; i < initCount; ++i)
			{
				if (itemFilter(static_cast<const Item&>(array[i])))
					continue;
				ItemTraits::Assign(memManager, std::move(array[i]), array[newCount]);
				++newCount;
			}
			size_t remCount = initCount - newCount;
			array.RemoveBack(remCount);
			return remCount;
		}
	};
}

} // namespace momo

namespace std
{
	template<typename A, typename I>
	struct iterator_traits<momo::internal::ArrayIndexIterator<A, I>>
		: public momo::internal::ArrayIndexIteratorTraitsStd<A, I>
	{
	};
} // namespace std

#endif // MOMO_INCLUDE_GUARD_ARRAY_UTILITY

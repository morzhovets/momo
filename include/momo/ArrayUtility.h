/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/ArrayUtility.h

\**********************************************************/

#pragma once

#include "ObjectManager.h"
#include "IteratorUtility.h"
#include "KeyUtility.h"

namespace momo
{

namespace internal
{
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
			typedef typename std::iterator_traits<ArrayIndexIterator>::iterator_concept IteratorConcept;
			if constexpr (std::is_base_of_v<std::contiguous_iterator_tag, IteratorConcept>)
				return mArray->GetItems() + mIndex;
			else
				return std::addressof((*mArray)[mIndex]);
		}

		friend bool operator==(ArrayIndexIterator iter1, ArrayIndexIterator iter2) noexcept = default;

		friend auto operator<=>(ArrayIndexIterator iter1, ArrayIndexIterator iter2)
		{
			MOMO_CHECK(iter1.mArray == iter2.mArray);
			return iter1.mIndex <=> iter2.mIndex;
		}

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
		template<conceptObjectCreator<Item> ItemCreator>
		explicit ArrayItemHandler(MemManager& memManager, FastMovableFunctor<ItemCreator> itemCreator)
			: mMemManager(&memManager)
		{
			std::move(itemCreator)(mItemBuffer.GetPtr());
		}

		ArrayItemHandler(const ArrayItemHandler&) = delete;

		~ArrayItemHandler() noexcept
		{
			if (mMemManager != nullptr)
				ItemTraits::Destroy(*mMemManager, std::addressof(Get()), 1);
		}

		ArrayItemHandler& operator=(const ArrayItemHandler&) = delete;

		Item& Get() noexcept
		{
			MOMO_ASSERT(mMemManager != nullptr);
			return mItemBuffer.Get();
		}

		void Release() noexcept
		{
			mMemManager = nullptr;
		}

	private:
		ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
		MemManager* mMemManager;
	};

	template<typename TArray>
	class ArrayInserter
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

		template<internal::conceptForwardIterator ArgIterator>
		static void InsertNogrow(Array& array, size_t index, ArgIterator begin, size_t count)
		{
			size_t initCount = array.GetCount();
			MOMO_CHECK(index <= initCount);
			MemManager& memManager = array.GetMemManager();
			MOMO_ASSERT(array.GetCapacity() >= initCount + count);
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
				typedef typename ItemTraits::template Creator<
					std::iter_reference_t<ArgIterator>> IterCreator;
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

		static void InsertNogrow(Array& array, size_t index, Item&& item)
		{
			InsertNogrow(array, index, std::make_move_iterator(std::addressof(item)), 1);
		}
	};

	template<typename TItem, typename TMemManager, typename TItemTraits, typename TSettings>
	class ArrayBase
	{
	public:
		typedef TItem Item;
		typedef TMemManager MemManager;
		typedef TItemTraits ItemTraits;
		typedef TSettings Settings;

	private:
		typedef ArrayItemHandler<ItemTraits> ItemHandler;

		template<typename RArray>
		using ArrayInserter = internal::ArrayInserter<std::decay_t<RArray>>;

	public:
		template<conceptMutableThis RArray, conceptObjectCreator<Item> ItemCreator>
		void InsertCrt(this RArray&& array, size_t index, ItemCreator itemCreator)
		{
			pvInsert(array, index,
				FastMovableFunctor<ItemCreator>(std::forward<ItemCreator>(itemCreator)));
		}

		template<conceptMutableThis RArray, typename... ItemArgs>
		//requires requires { typename ItemTraits::template Creator<ItemArgs...>; }
		void InsertVar(this RArray&& array, size_t index, ItemArgs&&... itemArgs)
		{
			array.InsertCrt(index, typename ItemTraits::template Creator<ItemArgs...>(
				array.GetMemManager(), std::forward<ItemArgs>(itemArgs)...));
		}

		template<conceptMutableThis RArray>
		void Insert(this RArray&& array, size_t index, Item&& item)
		{
			array.InsertVar(index, std::move(item));
		}

		template<conceptMutableThis RArray>
		void Insert(this RArray&& array, size_t index, const Item& item)
		{
			array.InsertVar(index, item);
		}

		template<conceptMutableThis RArray>
		void Insert(this RArray&& array, size_t index, size_t count, const Item& item)
		{
			typedef typename ItemTraits::template Creator<const Item&> ItemCreator;
			MemManager& memManager = array.GetMemManager();
			ItemHandler itemHandler(memManager, FastMovableFunctor(ItemCreator(memManager, item)));
			array.Reserve(array.GetCount() + count);
			ArrayInserter<RArray>::InsertNogrow(array, index, count, itemHandler.Get());
		}

		template<conceptMutableThis RArray,
			std::input_iterator ArgIterator, conceptSentinel<ArgIterator> ArgSentinel>
		void Insert(this RArray&& array, size_t index, ArgIterator begin, ArgSentinel end)
		{
			if constexpr (conceptForwardIterator<ArgIterator>)
			{
				size_t count = UIntMath<>::Dist(begin, end);
				array.Reserve(array.GetCount() + count);
				ArrayInserter<RArray>::InsertNogrow(array, index, begin, count);
			}
			else
			{
				typedef typename ItemTraits::template Creator<
					std::iter_reference_t<ArgIterator>> IterCreator;
				MemManager& memManager = array.GetMemManager();
				size_t count = 0;
				for (ArgIterator iter = std::move(begin); iter != end; (void)++iter, ++count)
					array.InsertCrt(index + count, IterCreator(memManager, *iter));
			}
		}

		template<conceptMutableThis RArray>
		void Insert(this RArray&& array, size_t index, std::initializer_list<Item> items)
		{
			array.Insert(index, items.begin(), items.end());
		}

		template<conceptMutableThis RArray>
		void Remove(this RArray&& array, size_t index, size_t count = 1)
		{
			size_t initCount = array.GetCount();
			MOMO_CHECK(index + count <= initCount);
			MemManager& memManager = array.GetMemManager();
			for (size_t i = index + count; i < initCount; ++i)
				ItemTraits::Assign(memManager, std::move(array[i]), array[i - count]);
			array.RemoveBack(count);
		}

		template<conceptMutableThis RArray, conceptObjectPredicate<Item> ItemFilter>
		size_t Remove(this RArray&& array, ItemFilter itemFilter)
		{
			size_t initCount = array.GetCount();
			size_t newCount = 0;
			while (newCount < initCount && !itemFilter(std::as_const(array[newCount])))
				++newCount;
			MemManager& memManager = array.GetMemManager();
			for (size_t i = newCount + 1; i < initCount; ++i)
			{
				if (itemFilter(std::as_const(array[i])))
					continue;
				ItemTraits::Assign(memManager, std::move(array[i]), array[newCount]);
				++newCount;
			}
			size_t remCount = initCount - newCount;
			array.RemoveBack(remCount);
			return remCount;
		}

		template<typename Array, typename ItemArg,
			conceptEqualFunc<Item, ItemArg> EqualFunc = std::equal_to<>>
		bool Contains(this const Array& array, const ItemArg& itemArg,
			EqualFunc equalFunc = EqualFunc())
		{
			FastCopyableFunctor<EqualFunc> fastEqualFunc(equalFunc);
			auto itemPred = [&itemArg, fastEqualFunc] (const Item& item)
				{ return fastEqualFunc(item, itemArg); };
			return std::any_of(array.GetBegin(), array.GetEnd(), FastCopyableFunctor(itemPred));
		}

		template<typename Array,
			conceptEqualFunc<Item> EqualFunc = std::equal_to<Item>>
		bool IsEqual(this const Array& array1, const std::type_identity_t<Array>& array2,
			EqualFunc equalFunc = EqualFunc())
		{
			return std::equal(array1.GetBegin(), array1.GetEnd(), array2.GetBegin(), array2.GetEnd(),
				FastCopyableFunctor<EqualFunc>(equalFunc));
		}

		template<typename Array,
			typename Comparer = TieComparer<typename Array::Item>>
		auto Compare(this const Array& array1, const std::type_identity_t<Array>& array2,
			Comparer comp = Comparer())
		{
			return std::lexicographical_compare_three_way(array1.GetBegin(), array1.GetEnd(),
				array2.GetBegin(), array2.GetEnd(), FastCopyableFunctor<Comparer>(comp));
		}

	protected:
		explicit ArrayBase() noexcept = default;

	private:
		template<typename Array, internal::conceptObjectCreator<Item> ItemCreator>
		static void pvInsert(Array& array, size_t index, FastMovableFunctor<ItemCreator> itemCreator)
		{
			ItemHandler itemHandler(array.GetMemManager(), std::move(itemCreator));
			array.Reserve(array.GetCount() + 1);
			ArrayInserter<Array>::InsertNogrow(array, index, std::move(itemHandler.Get()));
		}
	};

	template<typename TContainer>
	class BackInsertIteratorStd
	{
	private:
		typedef TContainer Container;
		typedef typename Container::Item Item;

	public:
		typedef Container container_type;
		typedef std::output_iterator_tag iterator_category;
		typedef ptrdiff_t difference_type;
		typedef void pointer;
		typedef void reference;
		typedef void value_type;

	public:
		explicit BackInsertIteratorStd(Container& cont) noexcept
			: container(&cont)
		{
		}

		template<conceptMutableThis RIterator,
			typename Iterator = std::decay_t<RIterator>>
		Iterator& operator=(this RIterator&& iter, Item&& item)
		{
			static_cast<BackInsertIteratorStd&>(iter).container->AddBack(std::move(item));
			return static_cast<Iterator&>(iter);
		}

		template<conceptMutableThis RIterator,
			typename Iterator = std::decay_t<RIterator>>
		Iterator& operator=(this RIterator&& iter, const Item& item)
		{
			static_cast<BackInsertIteratorStd&>(iter).container->AddBack(item);
			return static_cast<Iterator&>(iter);
		}

		template<conceptMutableThis RIterator,
			typename Iterator = std::decay_t<RIterator>>
		Iterator& operator*(this RIterator&& iter) noexcept
		{
			return static_cast<Iterator&>(iter);
		}

		template<conceptMutableThis RIterator,
			typename Iterator = std::decay_t<RIterator>>
		Iterator& operator++(this RIterator&& iter) noexcept
		{
			return static_cast<Iterator&>(iter);
		}

		template<conceptMutableThis RIterator,
			typename Iterator = std::decay_t<RIterator>>
		Iterator operator++(this RIterator&& iter, int) noexcept
		{
			return iter;
		}

	protected:
		Container* container;
	};
}

} // namespace momo

namespace std
{
	template<typename A, typename I>
	struct iterator_traits<momo::internal::ArrayIndexIterator<A, I>>
		: public momo::internal::IteratorTraitsStd<momo::internal::ArrayIndexIterator<A, I>,
			random_access_iterator_tag>
	{
	};

	template<typename A, typename I>
	requires requires (A& a) { { a.GetItems() } -> std::same_as<I*>; }
	struct iterator_traits<momo::internal::ArrayIndexIterator<A, I>>
		: public momo::internal::IteratorTraitsStd<momo::internal::ArrayIndexIterator<A, I>,
			random_access_iterator_tag, contiguous_iterator_tag>
	{
	};
} // namespace std

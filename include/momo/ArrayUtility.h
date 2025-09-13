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
	template<typename TQArray, typename TQItem,
		typename TSettings = typename TQArray::Settings>
	class ArrayIndexIterator : public ArrayIteratorBase
	{
	protected:
		typedef TQItem QItem;
		typedef TQArray QArray;
		typedef TSettings Settings;

	public:
		typedef QItem& Reference;
		typedef QItem* Pointer;

		typedef ArrayIndexIterator<const QArray, const QItem, Settings> ConstIterator;

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

		friend bool operator==(ArrayIndexIterator iter1, ArrayIndexIterator iter2) noexcept
		{
			return iter1.mArray == iter2.mArray && iter1.mIndex == iter2.mIndex;
		}

		friend auto operator<=>(ArrayIndexIterator iter1, ArrayIndexIterator iter2)
		{
			MOMO_CHECK(iter1.mArray == iter2.mArray);
			return iter1.mIndex <=> iter2.mIndex;
		}

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

		void Detach() noexcept
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
			if (count == 0)
				return;
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

		template<std::forward_iterator ArgIterator>
		static void InsertNogrow(Array& array, size_t index, ArgIterator begin, size_t count)
		{
			size_t initCount = array.GetCount();
			MOMO_CHECK(index <= initCount);
			MemManager& memManager = array.GetMemManager();
			MOMO_ASSERT(array.GetCapacity() >= initCount + count);
			if (count == 0)
				return;
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

	class ArrayBase : public Rangeable
	{
	public:
		template<conceptMutableThis RArray>
		void SetCount(this RArray&& array, size_t count)
		{
			typedef typename std::decay_t<RArray>::ItemTraits ItemTraits;
			typedef typename ItemTraits::template Creator<> ItemCreator;
			auto& memManager = array.GetMemManager();
			auto itemMultiCreator = [&memManager] (auto* newItem)
				{ (ItemCreator(memManager))(newItem); };
			array.SetCountCrt(count, itemMultiCreator);
		}

		template<conceptMutableThis RArray>
		void SetCount(this RArray&& array, size_t count,
			const typename std::decay_t<RArray>::Item& item)
		{
			typedef typename std::decay_t<RArray>::ItemTraits ItemTraits;
			typedef typename ItemTraits::template Creator<const typename ItemTraits::Item&> ItemCreator;
			auto& memManager = array.GetMemManager();
			auto itemMultiCreator = [&memManager, &item] (auto* newItem)
				{ ItemCreator(memManager, item)(newItem); };
			array.SetCountCrt(count, itemMultiCreator);
		}

		template<typename Array>
		bool IsEmpty(this const Array& array) noexcept
		{
			return array.GetCount() == 0;
		}

		template<conceptMutableThis RArray>
		void Shrink(this RArray&& array) noexcept(noexcept(array.Shrink(size_t{})))
		{
			array.Shrink(array.GetCount());
		}

		template<typename RArray>
		decltype(auto) GetBackItem(this RArray&& array, size_t revIndex = 0)
		{
			return array[array.GetCount() - 1 - revIndex];
		}

		//template<conceptMutableThis RArray, conceptObjectCreator<Item> ItemCreator>
		//void AddBackNogrowCrt(this RArray&& array, ItemCreator itemCreator)

		template<conceptMutableThis RArray, typename... ItemArgs>
		//requires requires { typename ItemTraits::template Creator<ItemArgs...>; }
		void AddBackNogrowVar(this RArray&& array, ItemArgs&&... itemArgs)
		{
			typedef typename std::decay_t<RArray>::ItemTraits ItemTraits;
			array.AddBackNogrowCrt(typename ItemTraits::template Creator<ItemArgs...>(
				array.GetMemManager(), std::forward<ItemArgs>(itemArgs)...));
		}

		template<conceptMutableThis RArray>
		void AddBackNogrow(this RArray&& array, typename std::decay_t<RArray>::Item&& item)
		{
			array.AddBackNogrowVar(std::move(item));
		}

		template<conceptMutableThis RArray>
		void AddBackNogrow(this RArray&& array, const typename std::decay_t<RArray>::Item& item)
		{
			array.AddBackNogrowVar(item);
		}

		//template<conceptMutableThis RArray, conceptObjectCreator<Item> ItemCreator>
		//void AddBackCrt(this RArray&& array, ItemCreator itemCreator)

		template<conceptMutableThis RArray, typename... ItemArgs>
		//requires requires { typename ItemTraits::template Creator<ItemArgs...>; }
		void AddBackVar(this RArray&& array, ItemArgs&&... itemArgs)
		{
			typedef typename std::decay_t<RArray>::ItemTraits ItemTraits;
			array.AddBackCrt(typename ItemTraits::template Creator<ItemArgs...>(
				array.GetMemManager(), std::forward<ItemArgs>(itemArgs)...));
		}

		template<conceptMutableThis RArray>
		void AddBack(this RArray&& array, typename std::decay_t<RArray>::Item&& item)
		{
			array.AddBackVar(std::move(item));
		}

		template<conceptMutableThis RArray>
		void AddBack(this RArray&& array, const typename std::decay_t<RArray>::Item& item)
		{
			array.AddBackVar(item);
		}

		template<conceptMutableThis RArray,
			conceptObjectCreator<typename std::decay_t<RArray>::Item> ItemCreator>
		void InsertCrt(this RArray&& array, size_t index, ItemCreator itemCreator)
		{
			pvInsert(array, index, FastMovableFunctor(std::forward<ItemCreator>(itemCreator)));
		}

		template<conceptMutableThis RArray, typename... ItemArgs>
		//requires requires { typename ItemTraits::template Creator<ItemArgs...>; }
		void InsertVar(this RArray&& array, size_t index, ItemArgs&&... itemArgs)
		{
			typedef typename std::decay_t<RArray>::ItemTraits ItemTraits;
			array.InsertCrt(index, typename ItemTraits::template Creator<ItemArgs...>(
				array.GetMemManager(), std::forward<ItemArgs>(itemArgs)...));
		}

		template<conceptMutableThis RArray>
		void Insert(this RArray&& array, size_t index, typename std::decay_t<RArray>::Item&& item)
		{
			array.InsertVar(index, std::move(item));
		}

		template<conceptMutableThis RArray>
		void Insert(this RArray&& array, size_t index, const typename std::decay_t<RArray>::Item& item)
		{
			array.InsertVar(index, item);
		}

		template<conceptMutableThis RArray>
		void Insert(this RArray&& array, size_t index, size_t count,
			const typename std::decay_t<RArray>::Item& item)
		{
			typedef typename std::decay_t<RArray>::ItemTraits ItemTraits;
			typedef typename ItemTraits::template Creator<const typename ItemTraits::Item&> ItemCreator;
			auto& memManager = array.GetMemManager();
			ArrayItemHandler<ItemTraits> itemHandler(memManager,
				FastMovableFunctor(ItemCreator(memManager, item)));
			array.Reserve(array.GetCount() + count);
			ArrayInserter<std::decay_t<RArray>>::InsertNogrow(array, index, count, itemHandler.Get());
		}

		template<conceptMutableThis RArray,
			std::input_iterator ArgIterator, conceptSentinel<ArgIterator> ArgSentinel>
		void Insert(this RArray&& array, size_t index, ArgIterator begin, ArgSentinel end)
		{
			if constexpr (std::forward_iterator<ArgIterator>)
			{
				size_t count = UIntMath<>::Dist(begin, end);
				array.Reserve(array.GetCount() + count);
				ArrayInserter<std::decay_t<RArray>>::InsertNogrow(array, index, begin, count);
			}
			else
			{
				typedef typename std::decay_t<RArray>::ItemTraits ItemTraits;
				typedef typename ItemTraits::template Creator<
					std::iter_reference_t<ArgIterator>> IterCreator;
				auto& memManager = array.GetMemManager();
				size_t count = 0;
				for (ArgIterator iter = std::move(begin); iter != end; (void)++iter, ++count)
					array.InsertCrt(index + count, IterCreator(memManager, *iter));
			}
		}

		template<conceptMutableThis RArray>
		void Insert(this RArray&& array, size_t index,
			std::initializer_list<typename std::decay_t<RArray>::Item> items)
		{
			array.Insert(index, items.begin(), items.end());
		}

		template<conceptMutableThis RArray>
		void Remove(this RArray&& array, size_t index, size_t count = 1)
		{
			typedef typename std::decay_t<RArray>::ItemTraits ItemTraits;
			typedef typename std::decay_t<RArray>::Settings Settings;
			size_t initCount = array.GetCount();
			MOMO_CHECK(index + count <= initCount);
			if (count == 0)
				return;
			auto& memManager = array.GetMemManager();
			for (size_t i = index + count; i < initCount; ++i)
				ItemTraits::Assign(memManager, std::move(array[i]), array[i - count]);
			array.RemoveBack(count);
		}

		template<conceptMutableThis RArray,
			conceptObjectPredicate<typename std::decay_t<RArray>::Item> ItemFilter>
		size_t Remove(this RArray&& array, ItemFilter itemFilter)
		{
			typedef typename std::decay_t<RArray>::ItemTraits ItemTraits;
			size_t initCount = array.GetCount();
			size_t newCount = 0;
			while (newCount < initCount && !itemFilter(std::as_const(array[newCount])))
				++newCount;
			auto& memManager = array.GetMemManager();
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
			conceptEqualComparer<typename Array::Item, ItemArg> ItemEqualComparer = std::equal_to<>>
		bool Contains(this const Array& array, const ItemArg& itemArg,
			ItemEqualComparer itemEqualComp = ItemEqualComparer())
		{
			FastCopyableFunctor fastItemEqualComp(itemEqualComp);
			auto itemPred = [&itemArg, fastItemEqualComp] (const typename Array::Item& item)
				{ return fastItemEqualComp(item, itemArg); };
			return std::any_of(array.GetBegin(), array.GetEnd(), FastCopyableFunctor(itemPred));
		}

		template<typename Array,
			conceptEqualComparer<typename Array::Item> ItemEqualComparer = std::equal_to<typename Array::Item>>
		bool IsEqual(this const Array& array1, const std::type_identity_t<Array>& array2,
			ItemEqualComparer itemEqualComp = ItemEqualComparer())
		{
			return std::equal(array1.GetBegin(), array1.GetEnd(), array2.GetBegin(), array2.GetEnd(),
				FastCopyableFunctor(itemEqualComp));
		}

		template<typename Array,
			typename ItemThreeComparer = TieThreeComparer<typename Array::Item>>
		auto Compare(this const Array& array1, const std::type_identity_t<Array>& array2,
			ItemThreeComparer itemThreeComp = ItemThreeComparer())
		{
			return std::lexicographical_compare_three_way(
				array1.GetBegin(), array1.GetEnd(), array2.GetBegin(), array2.GetEnd(),
				FastCopyableFunctor(itemThreeComp));
		}

	protected:
		explicit ArrayBase() noexcept = default;

	private:
		template<typename Array, internal::conceptObjectCreator<typename Array::Item> ItemCreator>
		static void pvInsert(Array& array, size_t index, FastMovableFunctor<ItemCreator> itemCreator)
		{
			ArrayItemHandler<typename Array::ItemTraits> itemHandler(array.GetMemManager(),
				std::move(itemCreator));
			array.Reserve(array.GetCount() + 1);
			ArrayInserter<Array>::InsertNogrow(array, index, std::move(itemHandler.Get()));
		}
	};

	template<typename TContainer>
	class BackInsertIteratorStdBase
	{
	private:
		typedef TContainer Container;
		typedef typename Container::Item Item;

		typedef std::back_insert_iterator<Container> BackInsertIteratorStd;

	public:
		typedef Container container_type;
		typedef std::output_iterator_tag iterator_category;
		typedef ptrdiff_t difference_type;
		typedef void pointer;
		typedef void reference;
		typedef void value_type;

	public:
		explicit BackInsertIteratorStdBase(Container& cont) noexcept
			: container(&cont)
		{
		}

		BackInsertIteratorStd& operator=(Item&& item)
		{
			container->AddBack(std::move(item));
			return **this;
		}

		BackInsertIteratorStd& operator=(const Item& item)
		{
			container->AddBack(item);
			return **this;
		}

		BackInsertIteratorStd& operator*() noexcept
		{
			static_assert(std::is_base_of_v<BackInsertIteratorStdBase, BackInsertIteratorStd>);
			return *static_cast<BackInsertIteratorStd*>(this);
		}

		BackInsertIteratorStd& operator++() noexcept
		{
			return **this;
		}

		BackInsertIteratorStd operator++(int) noexcept
		{
			return **this;
		}

	protected:
		Container* container;
	};
}

} // namespace momo

namespace std
{
	template<typename A, typename I, typename S>
	struct iterator_traits<momo::internal::ArrayIndexIterator<A, I, S>>
		: public momo::internal::IteratorTraitsStd<momo::internal::ArrayIndexIterator<A, I, S>,
			random_access_iterator_tag>
	{
	};

	template<typename A, typename I, typename S>
	requires requires (A& a) { { a.GetItems() } -> std::same_as<I*>; }
	struct iterator_traits<momo::internal::ArrayIndexIterator<A, I, S>>
		: public momo::internal::IteratorTraitsStd<momo::internal::ArrayIndexIterator<A, I, S>,
			random_access_iterator_tag, contiguous_iterator_tag>
	{
	};
} // namespace std

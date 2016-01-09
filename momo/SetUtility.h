/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/SetUtility.h

\**********************************************************/

#pragma once

#include "Utility.h"

namespace momo
{

namespace internal
{
	template<typename TKey, typename TItem>
	struct SetItemTraits
	{
		typedef TKey Key;
		typedef TItem Item;

		typedef internal::ObjectManager<Item> ItemManager;

		static const bool isNothrowAnywaySwappable = ItemManager::isNothrowAnywaySwappable;

		static const size_t alignment = ItemManager::alignment;

		template<typename... ItemArgs>
		using Creator = typename ItemManager::template Creator<ItemArgs...>;

		static const Key& GetKey(const Item& item) MOMO_NOEXCEPT
		{
			MOMO_STATIC_ASSERT((std::is_same<Item, Key>::value));
			return item;
		}

		static void Assign(Item&& srcItem, Item& dstItem)
		{
			dstItem = std::move(srcItem);
		}

		static void Assign(const Item& srcItem, Item& dstItem)
		{
			dstItem = srcItem;
		}

		static void Assign(Item&& srcItem, Item& midItem, Item& dstItem)
		{
			_Assign(std::move(srcItem), midItem, dstItem,
				internal::BoolConstant<ItemManager::isNothrowAnywayMoveAssignable>());
		}

		static void Assign(const Item& srcItem, Item& midItem, Item& dstItem)
		{
			_Assign(srcItem, midItem, dstItem,
				internal::BoolConstant<ItemManager::isNothrowAnywayCopyAssignable>());
		}

		template<typename Iterator, typename ItemCreator>
		static void RelocateCreate(Iterator srcBegin, Iterator dstBegin, size_t count,
			const ItemCreator& itemCreator, void* pobject)
		{
			ItemManager::RelocateCreate(srcBegin, dstBegin, count, itemCreator, pobject);
		}

	private:
		static void _Assign(Item&& srcItem, Item& midItem, Item& dstItem,
			std::true_type /*isNothrowAnywayMoveAssignable*/)
		{
			dstItem = std::move(midItem);
			ItemManager::AssignNothrowAnyway(std::move(srcItem), midItem);
		}

		static void _Assign(Item&& srcItem, Item& midItem, Item& dstItem,
			std::false_type /*isNothrowAnywayMoveAssignable*/)
		{
			dstItem = midItem;
			midItem = std::move(srcItem);
		}

		static void _Assign(const Item& srcItem, Item& midItem, Item& dstItem,
			std::true_type /*isNothrowAnywayCopyAssignable*/)
		{
			dstItem = std::move(midItem);
			ItemManager::AssignNothrowAnyway(srcItem, midItem);
		}

		static void _Assign(const Item& srcItem, Item& midItem, Item& dstItem,
			std::false_type /*isNothrowAnywayCopyAssignable*/)
		{
			dstItem = midItem;
			midItem = srcItem;
		}
	};

	template<typename TContainerTraits, typename TMemManager, typename TDetailParams>
	class SetCrew
	{
	public:
		typedef TContainerTraits ContainerTraits;
		typedef TMemManager MemManager;
		typedef TDetailParams DetailParams;

		MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<MemManager>::value);

	private:
		struct Data
		{
			size_t version;
			ContainerTraits containerTraits;
			MemManager memManager;
			DetailParams detailParams;
		};

	public:
		SetCrew(const ContainerTraits& containerTraits, MemManager&& memManager)
		{
			mData = (Data*)memManager.Allocate(sizeof(Data));
			mData->version = 0;
			new(&mData->memManager) MemManager(std::move(memManager));
			try
			{
				new(&mData->containerTraits) ContainerTraits(containerTraits);
				try
				{
					new(&mData->detailParams) DetailParams(mData->memManager);
				}
				catch (...)
				{
					mData->containerTraits.~ContainerTraits();
					throw;
				}
			}
			catch (...)
			{
				MemManager dataMemManager = std::move(mData->memManager);
				mData->memManager.~MemManager();
				dataMemManager.Deallocate(mData, sizeof(Data));
				throw;
			}
		}

		SetCrew(SetCrew&& crew) MOMO_NOEXCEPT
			: mData(nullptr)
		{
			Swap(crew);
		}

		SetCrew(const SetCrew&) = delete;

		~SetCrew() MOMO_NOEXCEPT
		{
			if (!_IsNull())
			{
				mData->detailParams.~DetailParams();
				mData->containerTraits.~ContainerTraits();
				MemManager memManager = std::move(mData->memManager);
				mData->memManager.~MemManager();
				memManager.Deallocate(mData, sizeof(Data));
			}
		}

		SetCrew& operator=(SetCrew&& crew) MOMO_NOEXCEPT
		{
			SetCrew(std::move(crew)).Swap(*this);
			return *this;
		}

		SetCrew& operator=(const SetCrew&) = delete;

		void Swap(SetCrew& crew) MOMO_NOEXCEPT
		{
			std::swap(mData, crew.mData);
		}

		const size_t& GetVersion() const MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->version;
		}

		size_t& GetVersion() MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->version;
		}

		const ContainerTraits& GetContainerTraits() const MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->containerTraits;
		}

		const MemManager& GetMemManager() const MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->memManager;
		}

		MemManager& GetMemManager() MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->memManager;
		}

		const DetailParams& GetDetailParams() const MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->detailParams;
		}

		DetailParams& GetDetailParams() MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return mData->detailParams;
		}

	private:
		bool _IsNull() const MOMO_NOEXCEPT
		{
			return mData == nullptr;
		}

	private:
		Data* mData;
	};
}

} // namespace momo

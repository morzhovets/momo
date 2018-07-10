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
	template<typename TKey, typename TItem, typename TMemManager>
	class SetItemTraits
	{
	public:
		typedef TKey Key;
		typedef TItem Item;
		typedef TMemManager MemManager;

	private:
		typedef ObjectManager<Item, MemManager> ItemManager;

	public:
		static const size_t alignment = ItemManager::alignment;

		static const bool isNothrowRelocatable = ItemManager::isNothrowRelocatable;

		template<typename... ItemArgs>
		using Creator = typename ItemManager::template Creator<ItemArgs...>;

	public:
		static const Key& GetKey(const Item& item) MOMO_NOEXCEPT
		{
			MOMO_STATIC_ASSERT((std::is_same<Item, Key>::value));
			return item;
		}

		static void Destroy(MemManager* memManager, Item& item) MOMO_NOEXCEPT
		{
			ItemManager::Destroyer::Destroy(memManager, item);
		}

		static void Relocate(MemManager* memManager, Item& srcItem, Item* dstItem)
			MOMO_NOEXCEPT_IF(isNothrowRelocatable)
		{
			ItemManager::Relocator::Relocate(memManager, srcItem, dstItem);
		}

		static void Replace(MemManager& memManager, Item& srcItem, Item& dstItem)
		{
			ItemManager::Replace(memManager, srcItem, dstItem);
		}

		static void ReplaceRelocate(MemManager& memManager, Item& srcItem, Item& midItem,
			Item* dstItem)
		{
			ItemManager::ReplaceRelocate(memManager, srcItem, midItem, dstItem);
		}

		template<typename KeyArg>
		static void AssignKey(MemManager& /*memManager*/, KeyArg&& keyArg, Item& item)
		{
			item = std::forward<KeyArg>(keyArg);
		}
	};

	template<typename TContainerTraits, typename TMemManager, bool tKeepVersion,
		bool tUsePtr = !std::is_nothrow_move_constructible<TContainerTraits>::value
			|| !std::is_nothrow_move_assignable<TContainerTraits>::value
			|| !std::is_empty<TMemManager>::value || tKeepVersion>
	class SetCrew;

	template<typename TContainerTraits, typename TMemManager, bool tKeepVersion>
	class SetCrew<TContainerTraits, TMemManager, tKeepVersion, true>
	{
	public:
		typedef TContainerTraits ContainerTraits;
		typedef TMemManager MemManager;

		MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<MemManager>::value);

		static const bool keepVersion = tKeepVersion;

	private:
		struct Data
		{
			size_t version;
			ContainerTraits containerTraits;
			MemManager memManager;
		};

	public:
		explicit SetCrew(const ContainerTraits& containerTraits, MemManager&& memManager)
		{
			mData = memManager.template Allocate<Data>(sizeof(Data));
			mData->version = 0;
			try
			{
				new(&mData->containerTraits) ContainerTraits(containerTraits);
			}
			catch (...)
			{
				memManager.Deallocate(mData, sizeof(Data));
				throw;
			}
			new(&mData->memManager) MemManager(std::move(memManager));
		}

		SetCrew(SetCrew&& crew) MOMO_NOEXCEPT
			: mData(nullptr)
		{
			Swap(crew);
		}

		SetCrew(const SetCrew&) = delete;

		~SetCrew() MOMO_NOEXCEPT
		{
			if (!pvIsNull())
			{
				mData->containerTraits.~ContainerTraits();
				MemManager memManager = std::move(GetMemManager());
				GetMemManager().~MemManager();
				memManager.Deallocate(mData, sizeof(Data));
			}
		}

		SetCrew& operator=(const SetCrew&) = delete;

		void Swap(SetCrew& crew) MOMO_NOEXCEPT
		{
			std::swap(mData, crew.mData);
		}

		const size_t* GetVersion() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!pvIsNull());
			return &mData->version;
		}

		void IncVersion() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!pvIsNull());
			++mData->version;
		}

		const ContainerTraits& GetContainerTraits() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!pvIsNull());
			return mData->containerTraits;
		}

		const MemManager& GetMemManager() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!pvIsNull());
			return mData->memManager;
		}

		MemManager& GetMemManager() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!pvIsNull());
			return mData->memManager;
		}

	private:
		bool pvIsNull() const MOMO_NOEXCEPT
		{
			return mData == nullptr;
		}

	private:
		Data* mData;
	};

	template<typename TContainerTraits, typename TMemManager, bool tKeepVersion>
	class SetCrew<TContainerTraits, TMemManager, tKeepVersion, false>
		: private TContainerTraits, private TMemManager
	{
	public:
		typedef TContainerTraits ContainerTraits;
		typedef TMemManager MemManager;

		MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<ContainerTraits>::value);
		MOMO_STATIC_ASSERT(std::is_nothrow_move_assignable<ContainerTraits>::value);
		MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<MemManager>::value);

		static const bool keepVersion = tKeepVersion;
		MOMO_STATIC_ASSERT(!keepVersion);

	public:
		explicit SetCrew(const ContainerTraits& containerTraits, MemManager&& memManager)
			: ContainerTraits(containerTraits),
			MemManager(std::move(memManager))
		{
		}

		SetCrew(SetCrew&& crew) MOMO_NOEXCEPT
			: ContainerTraits(std::move(crew.pvGetContainerTraits())),
			MemManager(std::move(crew.GetMemManager()))
		{
		}

		SetCrew(const SetCrew&) = delete;

		~SetCrew() MOMO_NOEXCEPT
		{
		}

		SetCrew& operator=(const SetCrew&) = delete;

		void Swap(SetCrew& crew) MOMO_NOEXCEPT
		{
			std::swap(pvGetContainerTraits(), crew.pvGetContainerTraits());
		}

		const size_t* GetVersion() const MOMO_NOEXCEPT
		{
			return nullptr;
		}

		void IncVersion() MOMO_NOEXCEPT
		{
		}

		const ContainerTraits& GetContainerTraits() const MOMO_NOEXCEPT
		{
			return *this;
		}

		const MemManager& GetMemManager() const MOMO_NOEXCEPT
		{
			return *this;
		}

		MemManager& GetMemManager() MOMO_NOEXCEPT
		{
			return *this;
		}

	private:
		ContainerTraits& pvGetContainerTraits() MOMO_NOEXCEPT
		{
			return *this;
		}
	};

	template<typename TItemTraits, typename TSettings>
	class SetExtractedItem
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TSettings Settings;

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

	public:
		explicit SetExtractedItem() MOMO_NOEXCEPT
			: mHasItem(false)
		{
		}

		template<typename Set>
		explicit SetExtractedItem(Set& set, typename Set::ConstIterator iter)
			: SetExtractedItem()
		{
			set.Remove(iter, *this);
		}

		SetExtractedItem(SetExtractedItem&& extractedItem)
			MOMO_NOEXCEPT_IF(ItemTraits::isNothrowRelocatable)
			: mHasItem(extractedItem.mHasItem)
		{
			if (mHasItem)
				ItemTraits::Relocate(nullptr, *&extractedItem.mItemBuffer, &mItemBuffer);
			extractedItem.mHasItem = false;
		}

		SetExtractedItem(const SetExtractedItem&) = delete;

		~SetExtractedItem() MOMO_NOEXCEPT
		{
			Clear();
		}

		SetExtractedItem& operator=(const SetExtractedItem&) = delete;

		bool IsEmpty() const MOMO_NOEXCEPT
		{
			return !mHasItem;
		}

		void Clear() MOMO_NOEXCEPT
		{
			if (mHasItem)
				ItemTraits::Destroy(nullptr, *&mItemBuffer);
			mHasItem = false;
		}

		const Item& GetItem() const
		{
			MOMO_CHECK(mHasItem);
			return *&mItemBuffer;
		}

		Item& GetItem()
		{
			MOMO_CHECK(mHasItem);
			return *&mItemBuffer;
		}

		template<typename ItemCreator>
		void Create(const ItemCreator& itemCreator)
		{
			MOMO_CHECK(!mHasItem);
			itemCreator(&mItemBuffer);
			mHasItem = true;
		}

		template<typename ItemRemover>
		void Remove(const ItemRemover& itemRemover)
		{
			MOMO_CHECK(mHasItem);
			itemRemover(*&mItemBuffer);
			mHasItem = false;
		}

	private:
		ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
		bool mHasItem;
	};
}

} // namespace momo

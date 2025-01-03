/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/SetUtility.h

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_SET_UTILITY
#define MOMO_INCLUDE_GUARD_SET_UTILITY

#include "ObjectManager.h"

namespace momo
{

namespace internal
{
	template<typename TKey, typename TMemManager>
	class SetItemTraits
	{
	public:
		typedef TKey Key;
		typedef TMemManager MemManager;
		typedef Key Item;

	private:
		typedef ObjectManager<Item, MemManager> ItemManager;

	public:
		static const size_t alignment = ItemManager::alignment;

		static const bool isNothrowRelocatable = ItemManager::isNothrowRelocatable;

		template<typename... ItemArgs>
		using Creator = typename ItemManager::template Creator<ItemArgs...>;

	public:
		static const Key& GetKey(const Item& item) noexcept
		{
			return item;
		}

		static void Destroy(MemManager* memManager, Item& item) noexcept
		{
			ItemManager::Destroyer::Destroy(memManager, item);
		}

		static void Relocate(MemManager* memManager, Item& srcItem, Item* dstItem)
			noexcept(isNothrowRelocatable)
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
			|| !std::is_empty<MemManagerPtr<TMemManager>>::value || tKeepVersion>
	class SetCrew;

	template<typename TContainerTraits, typename TMemManager, bool tKeepVersion>
	class SetCrew<TContainerTraits, TMemManager, tKeepVersion, true>
	{
	public:
		typedef TContainerTraits ContainerTraits;
		typedef TMemManager MemManager;

		static const bool keepVersion = tKeepVersion;

	private:
		typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

		class Data
		{
		public:
			explicit Data(const ContainerTraits& containerTraits)
				: version(0),
				containerTraits(containerTraits)
			{
			}

		public:
			size_t version;
			ContainerTraits containerTraits;
			ObjectBuffer<MemManager, alignof(MemManager)> memManagerBuffer;
		};

	public:
		explicit SetCrew(const ContainerTraits& containerTraits, MemManager&& memManager)
		{
			mData = MemManagerProxy::template AllocateCreate<Data>(memManager, containerTraits);
			::new(static_cast<void*>(mData->memManagerBuffer.GetPtr())) MemManager(std::move(memManager));
		}

		SetCrew(SetCrew&& crew) noexcept
			: mData(nullptr)
		{
			Swap(crew);
		}

		SetCrew(const SetCrew&) = delete;

		~SetCrew() noexcept
		{
			if (!pvIsNull())
			{
				MemManager* memManagerPtr = &GetMemManager();
				MemManager memManager = std::move(*memManagerPtr);
				memManagerPtr->~MemManager();
				mData->~Data();
				MemManagerProxy::Deallocate(memManager, mData, sizeof(Data));
			}
		}

		SetCrew& operator=(const SetCrew&) = delete;

		void Swap(SetCrew& crew) noexcept
		{
			std::swap(mData, crew.mData);
		}

		const size_t* GetVersion() const noexcept
		{
			MOMO_ASSERT(!pvIsNull());
			return &mData->version;
		}

		void IncVersion() noexcept
		{
			MOMO_ASSERT(!pvIsNull());
			++mData->version;
		}

		const ContainerTraits& GetContainerTraits() const noexcept
		{
			MOMO_ASSERT(!pvIsNull());
			return mData->containerTraits;
		}

		const MemManager& GetMemManager() const noexcept
		{
			MOMO_ASSERT(!pvIsNull());
			return mData->memManagerBuffer.Get();
		}

		MemManager& GetMemManager() noexcept
		{
			MOMO_ASSERT(!pvIsNull());
			return mData->memManagerBuffer.Get();
		}

	private:
		bool pvIsNull() const noexcept
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

		SetCrew(SetCrew&& crew) noexcept
			: ContainerTraits(std::move(crew.pvGetContainerTraits())),
			MemManager(std::move(crew.GetMemManager()))
		{
		}

		SetCrew(const SetCrew&) = delete;

		~SetCrew() = default;

		SetCrew& operator=(const SetCrew&) = delete;

		void Swap(SetCrew& crew) noexcept
		{
			std::swap(pvGetContainerTraits(), crew.pvGetContainerTraits());
		}

		const size_t* GetVersion() const noexcept
		{
			return nullptr;
		}

		void IncVersion() noexcept
		{
		}

		const ContainerTraits& GetContainerTraits() const noexcept
		{
			return *this;
		}

		const MemManager& GetMemManager() const noexcept
		{
			return *this;
		}

		MemManager& GetMemManager() noexcept
		{
			return *this;
		}

	private:
		ContainerTraits& pvGetContainerTraits() noexcept
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
		//typedef typename ItemTraits::MemManager MemManager;

	public:
		explicit SetExtractedItem() noexcept
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
			noexcept(ItemTraits::isNothrowRelocatable)
			: mHasItem(extractedItem.mHasItem)
		{
			if (mHasItem)
				ItemTraits::Relocate(nullptr, extractedItem.mItemBuffer.Get(), mItemBuffer.GetPtr());
			extractedItem.mHasItem = false;
		}

		SetExtractedItem(const SetExtractedItem&) = delete;

		~SetExtractedItem() noexcept
		{
			Clear();
		}

		SetExtractedItem& operator=(const SetExtractedItem&) = delete;

		bool IsEmpty() const noexcept
		{
			return !mHasItem;
		}

		void Clear() noexcept
		{
			if (mHasItem)
				ItemTraits::Destroy(nullptr, mItemBuffer.Get());
			mHasItem = false;
		}

		const Item& GetItem() const
		{
			MOMO_CHECK(mHasItem);
			return mItemBuffer.Get();
		}

		Item& GetItem()
		{
			MOMO_CHECK(mHasItem);
			return mItemBuffer.Get();
		}

		template<typename ItemCreator>
		void Create(ItemCreator&& itemCreator)
		{
			MOMO_CHECK(!mHasItem);
			std::forward<ItemCreator>(itemCreator)(mItemBuffer.GetPtr());
			mHasItem = true;
		}

		template<typename ItemRemover>
		void Remove(ItemRemover&& itemRemover)
		{
			MOMO_CHECK(mHasItem);
			std::forward<ItemRemover>(itemRemover)(mItemBuffer.Get());
			mHasItem = false;
		}

	private:
		ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
		bool mHasItem;
	};

	template<typename ArgIterator, typename Item,
		typename ArgReference = decltype(*std::declval<ArgIterator>())>
	struct IsSetArgIterator : public BoolConstant<std::is_reference<ArgReference>::value
		&& std::is_same<Item, typename std::decay<ArgReference>::type>::value>
	{
	};
}

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_SET_UTILITY

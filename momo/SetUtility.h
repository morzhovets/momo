/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/SetUtility.h

  namespace momo:
    concept conceptSetItemTraits

\**********************************************************/

#pragma once

#include "ObjectManager.h"

namespace momo
{

template<typename SetItemTraits, typename Key, typename MemManager>
concept conceptSetItemTraits =
	std::is_same_v<typename SetItemTraits::Key, Key> &&
	std::is_same_v<typename SetItemTraits::MemManager, MemManager> &&
	conceptObject<typename SetItemTraits::Item> &&
	requires (typename SetItemTraits::Item& item, MemManager& memManager)
	{
		typename std::integral_constant<size_t, SetItemTraits::alignment>;
		{ SetItemTraits::GetKey(std::as_const(item)) } noexcept -> std::same_as<const Key&>;
		{ SetItemTraits::Destroy(&memManager, item) } noexcept;
	} &&
	internal::ObjectAlignmenter<typename SetItemTraits::Item>::Check(SetItemTraits::alignment);

namespace internal
{
	template<conceptObject TKey, conceptMemManager TMemManager>
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

		static void Relocate(MemManager* /*srcMemManager*/, MemManager* dstMemManager,
			Item& srcItem, Item* dstItem) noexcept(isNothrowRelocatable)
		{
			ItemManager::Relocator::Relocate(dstMemManager, srcItem, dstItem);	//?
		}

		static void Replace(MemManager& memManager, Item& srcItem, Item& dstItem)
		{
			ItemManager::Replace(memManager, srcItem, dstItem);
		}

		static void ReplaceRelocate(MemManager& srcMemManager, Item& srcItem, Item& midItem,
			Item* dstItem)
		{
			ItemManager::ReplaceRelocate(srcMemManager, srcItem, midItem, dstItem);
		}

		template<typename KeyArg>
		static void AssignKey(MemManager& /*memManager*/, KeyArg&& keyArg, Item& item)
		{
			item = std::forward<KeyArg>(keyArg);
		}
	};

	template<std::copy_constructible TContainerTraits, conceptMemManager TMemManager,
		bool tKeepVersion>
	class SetCrew;

	template<std::copy_constructible TContainerTraits, conceptMemManager TMemManager,
		bool tKeepVersion>
	requires (!std::is_nothrow_move_constructible_v<TContainerTraits> ||
		!std::is_nothrow_move_assignable_v<TContainerTraits> ||
		!std::is_empty_v<MemManagerPtr<TMemManager>> || tKeepVersion)
	class SetCrew<TContainerTraits, TMemManager, tKeepVersion>
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
			explicit Data(const ContainerTraits& containerTraits, MemManager&& memManager)
				: version(0),
				containerTraits(containerTraits),
				memManager(std::move(memManager))
			{
			}

		public:
			size_t version;
			ContainerTraits containerTraits;
			MemManager memManager;
		};

	public:
		explicit SetCrew(const ContainerTraits& containerTraits, MemManager&& memManager)
		{
			mData = MemManagerProxy::template AllocateCreate<Data>(memManager,
				containerTraits, std::move(memManager));	//?
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
				MemManager memManager = std::move(GetMemManager());
				std::destroy_at(mData);
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
			return mData->memManager;
		}

		MemManager& GetMemManager() noexcept
		{
			MOMO_ASSERT(!pvIsNull());
			return mData->memManager;
		}

	private:
		bool pvIsNull() const noexcept
		{
			return mData == nullptr;
		}

	private:
		Data* mData;
	};

	template<std::copy_constructible TContainerTraits, conceptMemManager TMemManager,
		bool tKeepVersion>
	requires (std::is_nothrow_move_constructible_v<TContainerTraits> &&
		std::is_nothrow_move_assignable_v<TContainerTraits> &&
		std::is_empty_v<MemManagerPtr<TMemManager>> && !tKeepVersion)
	class SetCrew<TContainerTraits, TMemManager, tKeepVersion>
	{
	public:
		typedef TContainerTraits ContainerTraits;
		typedef TMemManager MemManager;

		static const bool keepVersion = tKeepVersion;

	public:
		explicit SetCrew(const ContainerTraits& containerTraits, MemManager&& memManager)
			: mContainerTraits(containerTraits),
			mMemManager(std::move(memManager))
		{
		}

		SetCrew(SetCrew&& crew) noexcept
			: mContainerTraits(std::move(crew.mContainerTraits)),
			mMemManager(std::move(crew.mMemManager))
		{
		}

		SetCrew(const SetCrew&) = delete;

		~SetCrew() = default;

		SetCrew& operator=(const SetCrew&) = delete;

		void Swap(SetCrew& crew) noexcept
		{
			std::swap(mContainerTraits, crew.mContainerTraits);
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
			return mContainerTraits;
		}

		const MemManager& GetMemManager() const noexcept
		{
			return mMemManager;
		}

		MemManager& GetMemManager() noexcept
		{
			return mMemManager;
		}

	private:
		[[no_unique_address]] ContainerTraits mContainerTraits;
		[[no_unique_address]] MemManager mMemManager;
	};

	template<typename TItemTraits, typename TSettings>
	class SetExtractedItem
	{
	protected:
		typedef TSettings Settings;

	public:
		typedef TItemTraits ItemTraits;
		typedef typename ItemTraits::Item Item;

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
				ItemTraits::Relocate(nullptr, nullptr, *&extractedItem.mItemBuffer, &mItemBuffer);
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

		template<std::invocable<Item*> ItemCreator>
		void Create(ItemCreator&& itemCreator)
		{
			MOMO_CHECK(!mHasItem);
			std::forward<ItemCreator>(itemCreator)(&mItemBuffer);
			mHasItem = true;
		}

		template<std::invocable<Item&> ItemRemover>
		void Remove(ItemRemover&& itemRemover)
		{
			MOMO_CHECK(mHasItem);
			std::forward<ItemRemover>(itemRemover)(*&mItemBuffer);
			mHasItem = false;
		}

	private:
		ObjectBuffer<Item, ItemTraits::alignment> mItemBuffer;
		bool mHasItem;
	};

	template<typename SetArgIterator, typename Item>
	concept conceptSetArgIterator = conceptInputIterator<SetArgIterator> &&
		std::is_reference_v<std::iter_reference_t<SetArgIterator>> &&
		std::is_same_v<Item, std::decay_t<std::iter_reference_t<SetArgIterator>>>;
}

} // namespace momo

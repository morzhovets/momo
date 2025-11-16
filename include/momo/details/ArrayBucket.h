/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/details/ArrayBucket.h

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../MemPool.h"

namespace momo
{

namespace internal
{
	template<typename TArrayBucketItemTraits>
	class ArrayBucketNestedArrayItemTraits
	{
	protected:
		typedef TArrayBucketItemTraits ArrayBucketItemTraits;
		typedef typename ArrayBucketItemTraits::MemManager ArrayBucketMemManager;

	public:
		typedef typename ArrayBucketItemTraits::Item Item;

		typedef MemManagerPtr<ArrayBucketMemManager> MemManager;

		static const size_t alignment = ArrayBucketItemTraits::alignment;

		static const bool isTriviallyRelocatable = ArrayBucketItemTraits::isTriviallyRelocatable;

		template<typename ItemArg>
		requires std::is_same_v<ItemArg, const Item&>
		class Creator
		{
		public:
			explicit Creator(MemManager& memManager, const Item& item) noexcept
				: mMemManager(memManager),
				mItem(item)
			{
			}

			void operator()(Item* newItem) const
			{
				ArrayBucketItemTraits::Copy(mMemManager.GetBaseMemManager(), mItem, newItem);
			}

		private:
			MemManager& mMemManager;
			const Item& mItem;
		};

	public:
		static void Destroy(MemManager& memManager, Item* items, size_t count) noexcept
		{
			ArrayBucketItemTraits::Destroy(memManager.GetBaseMemManager(), items, count);
		}

		static void Relocate(MemManager& memManager, Item* srcItems, Item* dstItems, size_t count)
		{
			ArrayBucketItemTraits::RelocateCreate(memManager.GetBaseMemManager(), srcItems,
				dstItems, count, FastMovableFunctor([] (Item*) noexcept {}), nullptr);
		}

		template<conceptObjectCreator<Item> ItemCreator>
		static void RelocateCreate(MemManager& memManager, Item* srcItems, Item* dstItems,
			size_t count, FastMovableFunctor<ItemCreator> itemCreator, Item* newItem)
		{
			ArrayBucketItemTraits::RelocateCreate(memManager.GetBaseMemManager(), srcItems,
				dstItems, count, std::move(itemCreator), newItem);
		}
	};

	template<size_t maxFastCount>
	concept conceptArrayBucketMaxFastCount = (0 < maxFastCount && maxFastCount < 16);

	template<typename TItemTraits, size_t tMaxFastCount,
		conceptMemPoolParamsBlockSizeAlignment TMemPoolParams, typename TArraySettings>
	requires conceptArrayBucketMaxFastCount<tMaxFastCount>
	class ArrayBucket
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;
		typedef TArraySettings ArraySettings;

		static const size_t maxFastCount = tMaxFastCount;

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef ArrayBounds<Item*> Bounds;
		typedef typename Bounds::ConstBounds ConstBounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef momo::Array<Item, MemManagerPtr,
			ArrayBucketNestedArrayItemTraits<ItemTraits>, ArraySettings> Array;

		static const size_t arrayAlignment = ObjectAlignmenter<Array>::alignment;
		typedef MemPoolParamsStatic<sizeof(Array) + arrayAlignment, arrayAlignment,
			MemPoolParams::blockCount, MemPoolParams::cachedFreeBlockCount> ArrayMemPoolParams;

		typedef MemPool<MemPoolParams, MemManagerPtr, NestedMemPoolSettings> FastMemPool;
		typedef MemPool<ArrayMemPoolParams, MemManagerPtr,
			NestedMemPoolSettings> ArrayMemPool;

		typedef BucketMemory<FastMemPool, std::byte*> FastMemory;
		typedef BucketMemory<ArrayMemPool, std::byte*> ArrayMemory;

	public:
		class Params
		{
		private:
			typedef NestedArrayIntCap<maxFastCount, FastMemPool, MemManagerDummy> FastMemPools;

		public:
			explicit Params(MemManager& memManager)
				: mArrayMemPool(MemManagerPtr(memManager))
			{
				for (size_t i = 1; i <= maxFastCount; ++i)
				{
					size_t blockSize = i * sizeof(Item) + ItemTraits::alignment;
					mFastMemPools.AddBackNogrow(
						FastMemPool(MemPoolParams(blockSize, ItemTraits::alignment),
						MemManagerPtr(memManager)));
				}
			}

			Params(const Params&) = delete;

			~Params() noexcept = default;

			Params& operator=(const Params&) = delete;

			void Clear() noexcept
			{
				if (mArrayMemPool.CanDeallocateAll())
					mArrayMemPool.DeallocateAll();
				for (FastMemPool& memPool : mFastMemPools)
				{
					if (memPool.CanDeallocateAll())
						memPool.DeallocateAll();
				}
			}

			MemManager& GetMemManager() noexcept
			{
				return mArrayMemPool.GetMemManager().GetBaseMemManager();
			}

			FastMemPool& GetFastMemPool(size_t memPoolIndex) noexcept
			{
				MOMO_ASSERT(memPoolIndex > 0);
				return mFastMemPools[memPoolIndex - 1];
			}

			ArrayMemPool& GetArrayMemPool() noexcept
			{
				return mArrayMemPool;
			}

		private:
			FastMemPools mFastMemPools;
			ArrayMemPool mArrayMemPool;
		};

	public:
		explicit ArrayBucket() noexcept
			: mPtr(nullptr)
		{
		}

		ArrayBucket(ArrayBucket&& bucket) noexcept
			: ArrayBucket()
		{
			Swap(bucket);
		}

		ArrayBucket(const ArrayBucket& bucket) = delete;

		explicit ArrayBucket(Params& params, const ArrayBucket& bucket)
		{
			MemManager& memManager = params.GetMemManager();
			ConstBounds bounds = bucket.GetBounds();
			size_t count = bounds.GetCount();
			if (count == 0)
			{
				mPtr = nullptr;
			}
			else if (count <= maxFastCount)
			{
				size_t memPoolIndex = pvGetFastMemPoolIndex(count);
				FastMemory memory(params.GetFastMemPool(memPoolIndex));
				Item* items = pvGetFastItems(memory.Get());
				size_t index = 0;
				for (internal::Finalizer fin(&ArrayBucket::pvDestroyExtraItems, memManager, items, index);
					fin; fin.Detach())
				{
					for (; index < count; ++index)
						ItemTraits::Copy(memManager, bounds[index], items + index);
				}
				pvSet(memory.Extract(), pvMakeState(memPoolIndex, count));
			}
			else
			{
				ArrayMemory memory(params.GetArrayMemPool());
				std::construct_at(pvGetArrayPtr(memory.Get()),
					bounds.GetBegin(), bounds.GetEnd(), MemManagerPtr(memManager));
				pvSet(memory.Extract(), uint8_t{0});
			}
		}

		~ArrayBucket() noexcept = default;

		ArrayBucket& operator=(ArrayBucket&& bucket) noexcept
		{
			ArrayBucket(std::move(bucket)).Swap(*this);
			return *this;
		}

		ArrayBucket& operator=(const ArrayBucket&) = delete;

		void Swap(ArrayBucket& bucket) noexcept
		{
			std::swap(mPtr, bucket.mPtr);
		}

		ConstBounds GetBounds() const noexcept
		{
			return pvGetBounds();
		}

		Bounds GetBounds() noexcept
		{
			return pvGetBounds();
		}

		void Clear(Params& params) noexcept
		{
			pvRemoveAll<true>(params);
		}

		template<conceptObjectCreator<Item> ItemCreator>
		void AddBackCrt(Params& params, FastMovableFunctor<ItemCreator> itemCreator)
		{
			if (mPtr == nullptr)
			{
				size_t newCount = 1;
				size_t newMemPoolIndex = pvGetFastMemPoolIndex(newCount);
				FastMemory memory(params.GetFastMemPool(newMemPoolIndex));
				std::move(itemCreator)(pvGetFastItems(memory.Get()));
				pvSet(memory.Extract(), pvMakeState(newMemPoolIndex, newCount));
			}
			else
			{
				size_t memPoolIndex = pvGetMemPoolIndex();
				if (memPoolIndex > 0)
				{
					size_t count = pvGetFastCount();
					MOMO_ASSERT(count <= memPoolIndex);
					if (count == memPoolIndex)
					{
						size_t newCount = count + 1;
						Item* items = pvGetFastItems();
						if (newCount <= maxFastCount)
						{
							size_t newMemPoolIndex = pvGetFastMemPoolIndex(newCount);
							FastMemory memory(params.GetFastMemPool(newMemPoolIndex));
							Item* newItems = pvGetFastItems(memory.Get());
							ItemTraits::RelocateCreate(params.GetMemManager(), items, newItems,
								count, std::move(itemCreator), newItems + count);
							params.GetFastMemPool(memPoolIndex).Deallocate(mPtr);
							pvSet(memory.Extract(), pvMakeState(newMemPoolIndex, newCount));
						}
						else
						{
							ArrayMemPool& arrayMemPool = params.GetArrayMemPool();
							ArrayMemory memory(arrayMemPool);
							Array array = Array::CreateCap(maxFastCount * 2,
								MemManagerPtr(arrayMemPool.GetMemManager()));
							Item* newItems = array.GetItems();
							ItemTraits::RelocateCreate(params.GetMemManager(), items, newItems,
								count, std::move(itemCreator), newItems + count);
							array.SetCountCrt(newCount, [] (Item* /*newItem*/) noexcept {});
							std::construct_at(pvGetArrayPtr(memory.Get()), std::move(array));
							params.GetFastMemPool(memPoolIndex).Deallocate(mPtr);
							pvSet(memory.Extract(), uint8_t{0});
						}
					}
					else
					{
						std::move(itemCreator)(pvGetFastItems() + count);
						pvSetState(pvGetState() + uint8_t{1});
					}
				}
				else
				{
					pvGetArray().AddBackCrt(std::move(itemCreator));
				}
			}
		}

		void RemoveBack(Params& params) noexcept
		{
			size_t count = GetBounds().GetCount();
			MOMO_ASSERT(count > 0);
			if (count == 1)
				return pvRemoveAll<false>(params);
			if (pvGetMemPoolIndex() > 0)
			{
				ItemTraits::Destroy(params.GetMemManager(), pvGetFastItems() + count - 1, 1);
				pvSetState(pvGetState() - uint8_t{1});
			}
			else
			{
				Array& array = pvGetArray();
				array.RemoveBack();
				if (2 < count && count <= array.GetCapacity() / 4)
					array.TryShrink(count * 2);
			}
		}

		void RemoveAll(Params& params) noexcept
		{
			pvRemoveAll<false>(params);
		}

	private:
		void pvSet(std::byte* ptr, uint8_t state) noexcept
		{
			mPtr = ptr;
			pvSetState(state);
		}

		uint8_t pvGetState() const noexcept
		{
			MOMO_ASSERT(mPtr != nullptr);
			return MemCopyer::FromBuffer<uint8_t>(mPtr);
		}

		void pvSetState(uint8_t state) noexcept
		{
			MOMO_ASSERT(mPtr != nullptr);
			MemCopyer::ToBuffer(state, mPtr);
		}

		static uint8_t pvMakeState(size_t memPoolIndex, size_t count) noexcept
		{
			return static_cast<uint8_t>((memPoolIndex << 4) | count);
		}

		static size_t pvGetFastMemPoolIndex(size_t count) noexcept
		{
			MOMO_ASSERT(0 < count && count <= maxFastCount);
			return count;
		}

		size_t pvGetMemPoolIndex() const noexcept
		{
			return size_t{pvGetState()} >> 4;
		}

		size_t pvGetFastCount() const noexcept
		{
			MOMO_ASSERT(pvGetMemPoolIndex() > 0);
			return size_t{pvGetState()} & 15;
		}

		Item* pvGetFastItems() const noexcept
		{
			MOMO_ASSERT(pvGetMemPoolIndex() > 0);
			return pvGetFastItems(mPtr);
		}

		static Item* pvGetFastItems(std::byte* ptr) noexcept
		{
			return PtrCaster::FromBytePtr<Item>(ptr + ItemTraits::alignment);
		}

		Array& pvGetArray() const noexcept
		{
			MOMO_ASSERT(pvGetMemPoolIndex() == 0);
			return *pvGetArrayPtr<true>(mPtr);
		}

		template<bool isWithinLifetime = false>
		static Array* pvGetArrayPtr(std::byte* ptr) noexcept
		{
			return PtrCaster::FromBytePtr<Array, isWithinLifetime, true>(ptr + arrayAlignment);
		}

		static void pvDestroyExtraItems(MemManager& memManager, Item* items, size_t& lastIndex) noexcept
		{
			ItemTraits::Destroy(memManager, items, lastIndex);
		}

		Bounds pvGetBounds() const noexcept
		{
			if (mPtr == nullptr)
			{
				return Bounds();
			}
			else if (pvGetMemPoolIndex() > 0)
			{
				return Bounds(pvGetFastItems(), pvGetFastCount());
			}
			else
			{
				Array& array = pvGetArray();
				return Bounds(array.GetItems(), array.GetCount());
			}
		}

		template<bool onClear>
		void pvRemoveAll(Params& params) noexcept
		{
			if (mPtr == nullptr)
				return;
			size_t memPoolIndex = pvGetMemPoolIndex();
			if (memPoolIndex > 0)
			{
				ItemTraits::Destroy(params.GetMemManager(), pvGetFastItems(), pvGetFastCount());
				FastMemPool& memPool = params.GetFastMemPool(memPoolIndex);
				if (!onClear || !memPool.CanDeallocateAll())
					memPool.Deallocate(mPtr);
			}
			else
			{
				std::destroy_at(&pvGetArray());
				ArrayMemPool& memPool = params.GetArrayMemPool();
				if (!onClear || !memPool.CanDeallocateAll())
					memPool.Deallocate(mPtr);
			}
			mPtr = nullptr;
		}

	private:
		std::byte* mPtr;
	};
}

} // namespace momo

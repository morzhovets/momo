/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

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

		static const bool isTriviallyRelocatable = false;	//?

		template<typename ItemArg>
		class Creator
		{
			MOMO_STATIC_ASSERT((std::is_same<ItemArg, const Item&>::value));

		public:
			explicit Creator(MemManager& memManager, const Item& item) MOMO_NOEXCEPT
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
		static void Destroy(MemManager& memManager, Item* items, size_t count) MOMO_NOEXCEPT
		{
			ArrayBucketItemTraits::Destroy(memManager.GetBaseMemManager(), items, count);
		}

		static void Relocate(MemManager& memManager, Item* srcItems, Item* dstItems, size_t count)
		{
			ArrayBucketItemTraits::RelocateCreate(memManager.GetBaseMemManager(), srcItems,
				dstItems, count, [] (Item*) {}, nullptr);
		}

		template<typename ItemCreator>
		static void RelocateCreate(MemManager& memManager, Item* srcItems, Item* dstItems,
			size_t count, const ItemCreator& itemCreator, Item* newItem)
		{
			ArrayBucketItemTraits::RelocateCreate(memManager.GetBaseMemManager(), srcItems,
				dstItems, count, itemCreator, newItem);
		}
	};

	template<typename TItemTraits, size_t tMaxFastCount, typename TMemPoolParams,
		typename TArraySettings>
	class ArrayBucket
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;
		typedef TArraySettings ArraySettings;

		static const size_t maxFastCount = tMaxFastCount;
		MOMO_STATIC_ASSERT(0 < maxFastCount && maxFastCount < 16);

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef BucketBounds<Item> Bounds;
		typedef typename Bounds::ConstBounds ConstBounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef ArrayBucketNestedArrayItemTraits<ItemTraits> ArrayItemTraits;

		typedef momo::Array<Item, MemManagerPtr, ArrayItemTraits,
			NestedArraySettings<ArraySettings>> Array;

		static const size_t arrayAlignment = std::alignment_of<Array>::value;

		typedef momo::MemPool<MemPoolParams, MemManagerPtr, NestedMemPoolSettings> MemPool;

		typedef BucketMemory<MemPool, unsigned char*> Memory;

	public:
		class Params
		{
		private:
			typedef ArrayIntCap<maxFastCount, MemPool, MemManagerDummy> MemPools;

		public:
			explicit Params(MemManager& memManager)
				: mArrayMemPool(MemPoolParams(sizeof(Array) + arrayAlignment, arrayAlignment),
					MemManagerPtr(memManager))
			{
				for (size_t i = 1; i <= maxFastCount; ++i)
				{
					size_t blockSize = i * sizeof(Item) + ItemTraits::alignment;
					mFastMemPools.AddBackNogrow(
						MemPool(MemPoolParams(blockSize, ItemTraits::alignment),
						MemManagerPtr(memManager)));
				}
			}

			Params(const Params&) = delete;

			~Params() MOMO_NOEXCEPT
			{
			}

			Params& operator=(const Params&) = delete;

			MemManager& GetMemManager() MOMO_NOEXCEPT
			{
				return mArrayMemPool.GetMemManager().GetBaseMemManager();
			}

			MemPool& GetFastMemPool(size_t memPoolIndex) MOMO_NOEXCEPT
			{
				MOMO_ASSERT(memPoolIndex > 0);
				return mFastMemPools[memPoolIndex - 1];
			}

			MemPool& GetArrayMemPool() MOMO_NOEXCEPT
			{
				return mArrayMemPool;
			}

		private:
			MemPools mFastMemPools;
			MemPool mArrayMemPool;
		};

	public:
		ArrayBucket() MOMO_NOEXCEPT
			: mPtr(nullptr)
		{
		}

		ArrayBucket(ArrayBucket&& bucket) MOMO_NOEXCEPT
			: mPtr(nullptr)
		{
			Swap(bucket);
		}

		ArrayBucket(const ArrayBucket& bucket) = delete;

		ArrayBucket(Params& params, const ArrayBucket& bucket)
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
				Memory memory(params.GetFastMemPool(memPoolIndex));
				Item* items = pvGetFastItems(memory.GetPointer());
				size_t index = 0;
				try
				{
					for (; index < count; ++index)
						ItemTraits::Copy(memManager, bounds[index], items + index);
				}
				catch (...)
				{
					ItemTraits::Destroy(memManager, items, index);
					throw;
				}
				pvSet(memory.Extract(), pvMakeState(memPoolIndex, count));
			}
			else
			{
				Memory memory(params.GetArrayMemPool());
				new(&pvGetArray(memory.GetPointer())) Array(bounds.GetBegin(),
					bounds.GetEnd(), MemManagerPtr(memManager));
				pvSet(memory.Extract(), (unsigned char)0);
			}
		}

		~ArrayBucket() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mPtr == nullptr);
		}

		ArrayBucket& operator=(ArrayBucket&& bucket) MOMO_NOEXCEPT
		{
			ArrayBucket(std::move(bucket)).Swap(*this);
			return *this;
		}

		ArrayBucket& operator=(const ArrayBucket& bucket) = delete;

		void Swap(ArrayBucket& bucket) MOMO_NOEXCEPT
		{
			std::swap(mPtr, bucket.mPtr);
		}

		ConstBounds GetBounds() const MOMO_NOEXCEPT
		{
			return pvGetBounds();
		}

		Bounds GetBounds() MOMO_NOEXCEPT
		{
			return pvGetBounds();
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (mPtr == nullptr)
				return;
			size_t memPoolIndex = pvGetMemPoolIndex();
			if (memPoolIndex > 0)
			{
				ItemTraits::Destroy(params.GetMemManager(), pvGetFastItems(), pvGetFastCount());
				params.GetFastMemPool(memPoolIndex).Deallocate(mPtr);
			}
			else
			{
				pvGetArray().~Array();
				params.GetArrayMemPool().Deallocate(mPtr);
			}
			mPtr = nullptr;
		}

		template<typename ItemCreator>
		void AddBackCrt(Params& params, const ItemCreator& itemCreator)
		{
			if (mPtr == nullptr)
			{
				size_t newCount = 1;
				size_t newMemPoolIndex = pvGetFastMemPoolIndex(newCount);
				Memory memory(params.GetFastMemPool(newMemPoolIndex));
				itemCreator(pvGetFastItems(memory.GetPointer()));
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
							Memory memory(params.GetFastMemPool(newMemPoolIndex));
							Item* newItems = pvGetFastItems(memory.GetPointer());
							ItemTraits::RelocateCreate(params.GetMemManager(), items, newItems,
								count, itemCreator, newItems + count);
							params.GetFastMemPool(memPoolIndex).Deallocate(mPtr);
							pvSet(memory.Extract(), pvMakeState(newMemPoolIndex, newCount));
						}
						else
						{
							MemPool& arrayMemPool = params.GetArrayMemPool();
							Memory memory(arrayMemPool);
							Array array = Array::CreateCap(maxFastCount * 2,
								MemManagerPtr(arrayMemPool.GetMemManager()));
							Item* newItems = array.GetItems();
							ItemTraits::RelocateCreate(params.GetMemManager(), items, newItems,
								count, itemCreator, newItems + count);
							array.SetCountCrt(newCount, [] (Item* /*newItem*/) { });
							new(&pvGetArray(memory.GetPointer())) Array(std::move(array));
							params.GetFastMemPool(memPoolIndex).Deallocate(mPtr);
							pvSet(memory.Extract(), (unsigned char)0);
						}
					}
					else
					{
						itemCreator(pvGetFastItems() + count);
						++*mPtr;
					}
				}
				else
				{
					pvGetArray().AddBackCrt(itemCreator);
				}
			}
		}

		void RemoveBack(Params& params) MOMO_NOEXCEPT
		{
			size_t count = GetBounds().GetCount();
			MOMO_ASSERT(count > 0);
			if (count == 1)
				return Clear(params);
			if (pvGetMemPoolIndex() > 0)
			{
				ItemTraits::Destroy(params.GetMemManager(), pvGetFastItems() + count - 1, 1);
				--*mPtr;
			}
			else
			{
				Array& array = pvGetArray();
				array.RemoveBack();
				if (4 < count && count < array.GetCapacity() / 2)
				{
					try
					{
						array.Shrink();
					}
					catch (...)
					{
						// no throw!
					}
				}
			}
		}

	private:
		void pvSet(unsigned char* ptr, unsigned char state) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(ptr != nullptr);
			mPtr = ptr;
			*mPtr = state;
		}

		static unsigned char pvMakeState(size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			return (unsigned char)((memPoolIndex << 4) | count);
		}

		static size_t pvGetFastMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(0 < count && count <= maxFastCount);
			return count;
		}

		size_t pvGetMemPoolIndex() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mPtr != nullptr);
			return (size_t)(*mPtr >> 4);
		}

		size_t pvGetFastCount() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(pvGetMemPoolIndex() > 0);
			return (size_t)(*mPtr & 15);
		}

		Item* pvGetFastItems() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(pvGetMemPoolIndex() > 0);
			return pvGetFastItems(mPtr);
		}

		static Item* pvGetFastItems(unsigned char* ptr) MOMO_NOEXCEPT
		{
			return reinterpret_cast<Item*>(ptr + ItemTraits::alignment);
		}

		Array& pvGetArray() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(pvGetMemPoolIndex() == 0);
			return pvGetArray(mPtr);
		}

		static Array& pvGetArray(unsigned char* ptr) MOMO_NOEXCEPT
		{
			return *reinterpret_cast<Array*>(ptr + arrayAlignment);
		}

		Bounds pvGetBounds() const MOMO_NOEXCEPT
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

	private:
		unsigned char* mPtr;
	};
}

} // namespace momo

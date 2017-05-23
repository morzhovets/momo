/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketLimP.h

  namespace momo:
    struct HashBucketLimP

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../MemPool.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, size_t tMaxCount, typename TMemPoolParams, bool tUsePtrState>
	class BucketLimP;

	template<typename TItemTraits, size_t tMaxCount, typename TMemPoolParams>
	class BucketLimP<TItemTraits, tMaxCount, TMemPoolParams, false>
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;

		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount < 16);

		static const bool usePtrState = false;

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef BucketBounds<Item> Bounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef momo::MemPool<MemPoolParams, MemManagerPtr, NestedMemPoolSettings> MemPool;

		typedef BucketMemory<MemPool, unsigned char*> Memory;

		static const uintptr_t ptrNull = UIntPtrConst::null;
		static const uintptr_t ptrNullWasFull = UIntPtrConst::invalid;

	public:
		class Params
		{
		private:
			typedef NestedArrayIntCap<maxCount, MemPool, MemManagerDummy> MemPools;

		public:
			explicit Params(MemManager& memManager)
			{
				for (size_t i = 1; i <= maxCount; ++i)
				{
					size_t blockSize = i * sizeof(Item) + ItemTraits::alignment;
					mMemPools.AddBackNogrow(MemPool(MemPoolParams(blockSize, ItemTraits::alignment),
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
				return mMemPools[0].GetMemManager().GetBaseMemManager();
			}

			MemPool& GetMemPool(size_t memPoolIndex) MOMO_NOEXCEPT
			{
				MOMO_ASSERT(memPoolIndex > 0);
				return mMemPools[memPoolIndex - 1];
			}

		private:
			MemPools mMemPools;
		};

	public:
		BucketLimP() MOMO_NOEXCEPT
			: mPtr(ptrNull)
		{
		}

		BucketLimP(const BucketLimP&) = delete;

		~BucketLimP() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(pvIsEmpty());
		}

		BucketLimP& operator=(const BucketLimP&) = delete;

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			if (pvIsEmpty())
				return Bounds();
			else
				return Bounds(pvGetItems(), pvGetCount());
		}

		bool TestIndex(size_t /*index*/, size_t /*hashCode*/) const MOMO_NOEXCEPT
		{
			return true;
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			if (pvIsEmpty())
				return false;
			return pvGetCount() == maxCount;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			if (mPtr == ptrNull)
				return false;
			if (mPtr == ptrNullWasFull)
				return true;
			return pvGetMemPoolIndex() == pvGetMemPoolIndex(maxCount);
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (!pvIsEmpty())
			{
				ItemTraits::Destroy(params.GetMemManager(), pvGetItems(), pvGetCount());
				params.GetMemPool(pvGetMemPoolIndex()).Deallocate(pvGetPtr());
			}
			mPtr = ptrNull;
		}

		template<typename ItemCreator>
		Item* AddBackCrt(Params& params, const ItemCreator& itemCreator, size_t /*hashCode*/)
		{
			if (pvIsEmpty())
			{
				size_t newCount = 1;
				size_t newMemPoolIndex = pvGetMemPoolIndex((mPtr == ptrNull) ? newCount : maxCount);
				Memory memory(params.GetMemPool(newMemPoolIndex));
				Item* newItems = pvGetItems(memory.GetPointer());
				itemCreator(newItems);
				pvSet(memory.Extract(), newMemPoolIndex, newCount);
				return newItems;
			}
			else
			{
				size_t memPoolIndex = pvGetMemPoolIndex();
				size_t count = pvGetCount();
				MOMO_ASSERT(count <= memPoolIndex);
				MOMO_ASSERT(count < maxCount);
				if (count == memPoolIndex)
				{
					size_t newCount = count + 1;
					size_t newMemPoolIndex = pvGetMemPoolIndex(newCount);
					Memory memory(params.GetMemPool(newMemPoolIndex));
					Item* newItems = pvGetItems(memory.GetPointer());
					ItemTraits::RelocateCreate(params.GetMemManager(), pvGetItems(), newItems,
						count, itemCreator, newItems + count);
					params.GetMemPool(memPoolIndex).Deallocate(pvGetPtr());
					pvSet(memory.Extract(), newMemPoolIndex, newCount);
					return newItems + count;
				}
				else
				{
					Item* items = pvGetItems();
					itemCreator(items + count);
					++*pvGetPtr();
					return items + count;
				}
			}
		}

		void DecCount(Params& params) MOMO_NOEXCEPT
		{
			size_t count = pvGetCount();
			MOMO_ASSERT(count > 0);
			if (count == 1)
			{
				size_t memPoolIndex = pvGetMemPoolIndex();
				params.GetMemPool(memPoolIndex).Deallocate(pvGetPtr());
				mPtr = (memPoolIndex < pvGetMemPoolIndex(maxCount)) ? ptrNull : ptrNullWasFull;
			}
			else
			{
				--*pvGetPtr();
			}
		}

	private:
		bool pvIsEmpty() const MOMO_NOEXCEPT
		{
			return mPtr == ptrNull || mPtr == ptrNullWasFull;
		}

		unsigned char* pvGetPtr() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!pvIsEmpty());
			return reinterpret_cast<unsigned char*>(mPtr);
		}

		void pvSet(unsigned char* ptr, size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(ptr != nullptr);
			mPtr = reinterpret_cast<uintptr_t>(ptr);
			*ptr = (unsigned char)((memPoolIndex << 4) | count);
		}

		static size_t pvGetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(0 < count && count <= maxCount);
			return count;
		}

		size_t pvGetMemPoolIndex() const MOMO_NOEXCEPT
		{
			return (size_t)(*pvGetPtr() >> 4);
		}

		size_t pvGetCount() const MOMO_NOEXCEPT
		{
			return (size_t)(*pvGetPtr() & 15);
		}

		Item* pvGetItems() const MOMO_NOEXCEPT
		{
			return pvGetItems(pvGetPtr());
		}

		static Item* pvGetItems(unsigned char* ptr) MOMO_NOEXCEPT
		{
			return reinterpret_cast<Item*>(ptr + ItemTraits::alignment);
		}

	private:
		uintptr_t mPtr;
	};

	template<typename TItemTraits, size_t tMaxCount, typename TMemPoolParams>
	class BucketLimP<TItemTraits, tMaxCount, TMemPoolParams, true>
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;

		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount < 16);

		static const bool usePtrState = true;

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef BucketBounds<Item> Bounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef momo::MemPool<MemPoolParams, MemManagerPtr, NestedMemPoolSettings> MemPool;

		typedef BucketMemory<MemPool, Item*> Memory;

		static const size_t minItemAlignment = (maxCount <= 1) ? 1 : (maxCount <= 2) ? 2
			: (maxCount <= 4) ? 4 : (maxCount <= 8) ? 8 : 16;
		static const size_t itemAlignment = (ItemTraits::alignment < minItemAlignment)
			? minItemAlignment : ItemTraits::alignment;

		static const bool skipOddMemPools = (maxCount > 1 && sizeof(Item) <= itemAlignment);	//?
		static const uintptr_t modMemPoolIndex =
			(uintptr_t)minItemAlignment / (skipOddMemPools ? 2 : 1);

		static const uintptr_t stateNull = UIntPtrConst::null;
		static const uintptr_t stateNullWasFull = UIntPtrConst::invalid;

	public:
		class Params
		{
		private:
			typedef NestedArrayIntCap<maxCount, MemPool, MemManagerDummy> MemPools;

		public:
			explicit Params(MemManager& memManager)
			{
				for (size_t i = 1; i <= maxCount + (skipOddMemPools ? 1 : 0); ++i)
				{
					if (skipOddMemPools && i % 2 == 1)
						continue;
					size_t blockAlignment = UIntMath<size_t>::Ceil(i * (size_t)modMemPoolIndex,
						itemAlignment);
					mMemPools.AddBackNogrow(MemPool(MemPoolParams(i * sizeof(Item), blockAlignment),
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
				return mMemPools[0].GetMemManager().GetBaseMemManager();
			}

			MemPool& GetMemPool(size_t memPoolIndex) MOMO_NOEXCEPT
			{
				MOMO_ASSERT(memPoolIndex > 0);
				return mMemPools[(memPoolIndex - 1) / (skipOddMemPools ? 2 : 1)];
			}

		private:
			MemPools mMemPools;
		};

	public:
		BucketLimP() MOMO_NOEXCEPT
			: mPtrState(stateNull)
		{
		}

		BucketLimP(const BucketLimP&) = delete;

		~BucketLimP() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(pvIsEmpty());
		}

		BucketLimP& operator=(const BucketLimP&) = delete;

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			if (pvIsEmpty())
				return Bounds();
			return pvGetBounds();
		}

		bool TestIndex(size_t /*index*/, size_t /*hashCode*/) const MOMO_NOEXCEPT
		{
			return true;
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			if (pvIsEmpty())
				return false;
			return pvGetBounds().GetCount() == maxCount;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			if (mPtrState == stateNull)
				return false;
			if (mPtrState == stateNullWasFull)
				return true;
			return pvGetMemPoolIndex() == pvGetMemPoolIndex(maxCount);
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (!pvIsEmpty())
			{
				Bounds bounds = pvGetBounds();
				Item* items = bounds.GetBegin();
				ItemTraits::Destroy(params.GetMemManager(), items, bounds.GetCount());
				params.GetMemPool(pvGetMemPoolIndex()).Deallocate(items);
			}
			mPtrState = stateNull;
		}

		template<typename ItemCreator>
		Item* AddBackCrt(Params& params, const ItemCreator& itemCreator, size_t /*hashCode*/)
		{
			if (pvIsEmpty())
			{
				size_t newCount = 1;
				size_t newMemPoolIndex =
					pvGetMemPoolIndex((mPtrState == stateNull) ? newCount : maxCount);
				Memory memory(params.GetMemPool(newMemPoolIndex));
				Item* newItems = memory.GetPointer();
				itemCreator(newItems);
				pvSet(memory.Extract(), newMemPoolIndex, newCount);
				return newItems;
			}
			else
			{
				size_t memPoolIndex = pvGetMemPoolIndex();
				Bounds bounds = pvGetBounds();
				size_t count = bounds.GetCount();
				Item* items = bounds.GetBegin();
				MOMO_ASSERT(count <= memPoolIndex);
				MOMO_ASSERT(count < maxCount);
				if (count == memPoolIndex)
				{
					size_t newCount = count + 1;
					size_t newMemPoolIndex = pvGetMemPoolIndex(newCount);
					Memory memory(params.GetMemPool(newMemPoolIndex));
					Item* newItems = memory.GetPointer();
					ItemTraits::RelocateCreate(params.GetMemManager(), items, newItems, count,
						itemCreator, newItems + count);
					params.GetMemPool(memPoolIndex).Deallocate(items);
					pvSet(memory.Extract(), newMemPoolIndex, newCount);
					return newItems + count;
				}
				else
				{
					itemCreator(items + count);
					mPtrState += modMemPoolIndex;
					return items + count;
				}
			}
		}

		void DecCount(Params& params) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!pvIsEmpty());
			Bounds bounds = pvGetBounds();
			if (bounds.GetCount() == 1)
			{
				size_t memPoolIndex = pvGetMemPoolIndex();
				params.GetMemPool(memPoolIndex).Deallocate(bounds.GetBegin());
				mPtrState = (memPoolIndex < pvGetMemPoolIndex(maxCount))
					? stateNull : stateNullWasFull;
			}
			else
			{
				mPtrState -= modMemPoolIndex;
			}
		}

	private:
		bool pvIsEmpty() const MOMO_NOEXCEPT
		{
			return mPtrState == stateNull || mPtrState == stateNullWasFull;
		}

		void pvSet(Item* items, size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			mPtrState = reinterpret_cast<uintptr_t>(items) + (uintptr_t)(count - 1) * modMemPoolIndex
				+ (uintptr_t)memPoolIndex / (skipOddMemPools ? 2 : 1) - 1;
		}

		static size_t pvGetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(0 < count && count <= maxCount);
			return count + (skipOddMemPools ? count % 2 : 0);
		}

		size_t pvGetMemPoolIndex() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!pvIsEmpty());
			return (size_t)((mPtrState % modMemPoolIndex) + 1) * (skipOddMemPools ? 2 : 1);
		}

		Bounds pvGetBounds() const MOMO_NOEXCEPT
		{
			uintptr_t memPoolIndex = (uintptr_t)pvGetMemPoolIndex();
			uintptr_t ptrCount = mPtrState / modMemPoolIndex;
			uintptr_t mod = UIntMath<uintptr_t>::Ceil(memPoolIndex,
				(uintptr_t)itemAlignment / modMemPoolIndex);
			uintptr_t count1 = UIntMath<uintptr_t>::DivBySmall(ptrCount, mod).remainder;
			MOMO_ASSERT(count1 < memPoolIndex);
			Item* items = reinterpret_cast<Item*>((ptrCount - count1) * modMemPoolIndex);
			return Bounds(items, (size_t)count1 + 1);
		}

	private:
		uintptr_t mPtrState;
	};
}

template<size_t tMaxCount = sizeof(void*),
	typename TMemPoolParams = MemPoolParams<>,
	bool tUsePtrState = true>
struct HashBucketLimP : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;
	static const bool usePtrState = tUsePtrState;

	typedef TMemPoolParams MemPoolParams;

private:
	template<typename ItemTraits>
	struct Bucketer
	{
	private:
		static const size_t size = sizeof(typename ItemTraits::Item);
		static const size_t alignment = ItemTraits::alignment;

		static const bool usePtrState = HashBucketLimP::usePtrState && ((maxCount <= 1)
			|| (maxCount <= 2 && size % 2 == 0 && (size > 2 || alignment == 2))
			|| (maxCount <= 4 && size % 4 == 0 && (size > 4 || alignment == 4))
			|| (maxCount <= 8 && size % 8 == 0 && (size > 8 || alignment == 8))
			|| (maxCount <= 16 && size % 16 == 0 && (size > 16 || alignment == 16)));

	public:
		typedef internal::BucketLimP<ItemTraits, maxCount, MemPoolParams, usePtrState> Bucket;
	};

public:
	template<typename ItemTraits>
	using Bucket = typename Bucketer<ItemTraits>::Bucket;
};

} // namespace momo

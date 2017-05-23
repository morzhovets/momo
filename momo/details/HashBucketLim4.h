/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketLim4.h

  namespace momo:
    struct HashBucketLim4

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../MemPool.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, size_t tLogMaxCount, size_t tMemPoolBlockCount>
	class BucketLim4
	{
	protected:
		typedef TItemTraits ItemTraits;

		static const size_t logMaxCount = tLogMaxCount;
		MOMO_STATIC_ASSERT(0 < logMaxCount && logMaxCount <= 4);	//?

		static const size_t memPoolBlockCount = tMemPoolBlockCount;

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef BucketBounds<Item> Bounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef MemPoolUInt32<memPoolBlockCount, MemManagerPtr> MemPool;

		typedef BucketMemory<MemPool, uint32_t, MemPool::nullPtr> Memory;
		
		static const size_t maxCount = 1 << logMaxCount;

		static const uint32_t stateNull = ((uint32_t)1 << (32 - logMaxCount)) - 1;
		static const uint32_t stateNullWasFull = stateNull - 1;

		struct Data
		{
			uint32_t pointer;
			size_t count;
		};

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
					auto memPoolCreator = [i, &memManager] (MemPool* newMemPool)
					{
						size_t maxTotalBlockCount = (i == 1)
							? ((size_t)1 << (32 - logMaxCount)) - 2
							: ((size_t)1 << (32 - logMaxCount)) / i;
						new(newMemPool) MemPool(i * sizeof(Item), MemManagerPtr(memManager),
							maxTotalBlockCount);
					};
					mMemPools.AddBackNogrowCrt(memPoolCreator);
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
		BucketLim4() MOMO_NOEXCEPT
			: mPtrState(stateNull)
		{
		}

		BucketLim4(const BucketLim4&) = delete;

		~BucketLim4() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(pvIsEmpty());
		}

		BucketLim4& operator=(const BucketLim4&) = delete;

		Bounds GetBounds(Params& params) MOMO_NOEXCEPT
		{
			if (pvIsEmpty())
				return Bounds();
			Data data = pvGetData();
			MemPool& memPool = params.GetMemPool(pvGetMemPoolIndex());
			Item* items = memPool.template GetRealPointer<Item>(data.pointer);
			return Bounds(items, data.count);
		}

		bool TestIndex(size_t /*index*/) const MOMO_NOEXCEPT
		{
			return true;
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			if (pvIsEmpty())
				return false;
			return pvGetData().count == maxCount;
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
				Data data = pvGetData();
				uint32_t ptr = data.pointer;
				size_t memPoolIndex = pvGetMemPoolIndex();
				MemPool& memPool = params.GetMemPool(memPoolIndex);
				Item* items = memPool.template GetRealPointer<Item>(ptr);
				ItemTraits::Destroy(params.GetMemManager(), items, data.count);
				memPool.Deallocate(ptr);
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
				MemPool& newMemPool = params.GetMemPool(newMemPoolIndex);
				Memory memory(newMemPool);
				Item* newItems = newMemPool.template GetRealPointer<Item>(memory.GetPointer());
				itemCreator(newItems);
				pvSet(memory.Extract(), newMemPoolIndex, newCount);
				return newItems;
			}
			else
			{
				Data data = pvGetData();
				size_t count = data.count;
				uint32_t ptr = data.pointer;
				size_t memPoolIndex = pvGetMemPoolIndex();
				MOMO_ASSERT(count <= memPoolIndex);
				MOMO_ASSERT(count < maxCount);
				MemPool& memPool = params.GetMemPool(memPoolIndex);
				Item* items = memPool.template GetRealPointer<Item>(ptr);
				if (count == memPoolIndex)
				{
					size_t newCount = count + 1;
					size_t newMemPoolIndex = pvGetMemPoolIndex(newCount);
					MemPool& newMemPool = params.GetMemPool(newMemPoolIndex);
					Memory memory(newMemPool);
					Item* newItems = newMemPool.template GetRealPointer<Item>(memory.GetPointer());
					ItemTraits::RelocateCreate(params.GetMemManager(), items, newItems, count,
						itemCreator, newItems + count);
					memPool.Deallocate(ptr);
					pvSet(memory.Extract(), newMemPoolIndex, newCount);
					return newItems + count;
				}
				else
				{
					itemCreator(items + count);
					++mPtrState;
					return items + count;
				}
			}
		}

		void DecCount(Params& params) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!pvIsEmpty());
			Data data = pvGetData();
			if (data.count == 1)
			{
				size_t memPoolIndex = pvGetMemPoolIndex();
				params.GetMemPool(memPoolIndex).Deallocate(data.pointer);
				mPtrState = (memPoolIndex < pvGetMemPoolIndex(maxCount))
					? stateNull : stateNullWasFull;
			}
			else
			{
				--mPtrState;
			}
		}

	private:
		bool pvIsEmpty() const MOMO_NOEXCEPT
		{
			return mPtrState == stateNull || mPtrState == stateNullWasFull;
		}

		void pvSet(uint32_t ptr, size_t memPoolIndex, size_t count)
		{
			mPtrState = (uint32_t)(((memPoolIndex - 1) << (32 - logMaxCount))
				+ (size_t)ptr * memPoolIndex + count - 1);
		}

		static size_t pvGetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(0 < count && count <= maxCount);
			return count;
		}

		size_t pvGetMemPoolIndex() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!pvIsEmpty());
			return (size_t)(mPtrState >> (32 - logMaxCount)) + 1;
		}

		Data pvGetData() const MOMO_NOEXCEPT
		{
			UIntMath<uint32_t>::DivResult divRes = UIntMath<uint32_t>::DivBySmall(
				mPtrState & stateNull, (uint32_t)pvGetMemPoolIndex());
			Data data;
			data.pointer = divRes.quotient;
			data.count = (size_t)divRes.remainder + 1;
			return data;
		}

	private:
		uint32_t mPtrState;
	};
}

template<size_t tLogMaxCount = 2,
	size_t tMemPoolBlockCount = MemPoolConst::defaultBlockCount>
struct HashBucketLim4 : public internal::HashBucketBase<1 << tLogMaxCount>
{
	static const size_t logMaxCount = tLogMaxCount;
	static const size_t memPoolBlockCount = tMemPoolBlockCount;

	template<typename ItemTraits>
	using Bucket = internal::BucketLim4<ItemTraits, logMaxCount, memPoolBlockCount>;
};

} // namespace momo

/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketLimP1.h

  namespace momo:
    struct HashBucketLimP1

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../MemPool.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, size_t tMaxCount, typename TMemPoolParams, size_t tAlignment>
	class BucketLimP1
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;

		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount < 16);

		static const size_t alignment = tAlignment;

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef Item* Iterator;
		typedef BucketBounds<Item> Bounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef momo::MemPool<MemPoolParams, MemManagerPtr, NestedMemPoolSettings> MemPool;

		typedef BucketMemory<MemPool, Item*> Memory;

	public:
		class Params
		{
		public:
			static const bool skipFirstMemPool =
				(maxCount > 1 && ItemTraits::alignment == sizeof(Item));	//?

		private:
			typedef NestedArrayIntCap<maxCount, MemPool, MemManagerDummy> MemPools;

			static const size_t minMemPoolIndex = skipFirstMemPool ? 2 : 1;

		public:
			explicit Params(MemManager& memManager)
			{
				for (size_t i = minMemPoolIndex; i <= maxCount; ++i)
				{
					size_t blockSize = i * sizeof(Item);
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
				MOMO_ASSERT(memPoolIndex >= minMemPoolIndex);
				return mMemPools[memPoolIndex - minMemPoolIndex];
			}

		private:
			MemPools mMemPools;
		};

	public:
		BucketLimP1() MOMO_NOEXCEPT
		{
			pvSet(nullptr, pvGetMemPoolIndex(1), 0);
		}

		BucketLimP1(const BucketLimP1&) = delete;

		~BucketLimP1() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(pvGetItems() == nullptr);
		}

		BucketLimP1& operator=(const BucketLimP1&) = delete;

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return Bounds(pvGetItems(), pvGetCount());
		}

		template<typename Predicate>
		Iterator Find(Params& /*params*/, const Predicate& pred, size_t /*hashCode*/)
		{
			size_t count = pvGetCount();
			Item* items = pvGetItems();
			for (size_t i = 0; i < count; ++i)
			{
				if (pred(items[i]))
					return items + i;
			}
			return nullptr;
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return pvGetCount() == maxCount;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return pvGetMemPoolIndex() == pvGetMemPoolIndex(maxCount);
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			Item* items = pvGetItems();
			if (items != nullptr)
			{
				ItemTraits::Destroy(params.GetMemManager(), items, pvGetCount());
				params.GetMemPool(pvGetMemPoolIndex()).Deallocate(items);
			}
			pvSet(nullptr, pvGetMemPoolIndex(1), 0);
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& params, const ItemCreator& itemCreator, size_t /*hashCode*/)
		{
			Item* items = pvGetItems();
			if (items == nullptr)
			{
				size_t newCount = 1;
				size_t newMemPoolIndex = pvGetMemPoolIndex();
				Memory memory(params.GetMemPool(newMemPoolIndex));
				Item* newItems = memory.GetPointer();
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
					++mState;
					return items + count;
				}
			}
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& params, Iterator iter, const ItemReplacer& itemReplacer)
		{
			size_t count = pvGetCount();
			MOMO_ASSERT(count > 0);
			Item* items = pvGetItems();
			if (count == 1)
			{
				MOMO_ASSERT(iter == items);
				itemReplacer(*items, *items);
				size_t memPoolIndex = pvGetMemPoolIndex();
				params.GetMemPool(memPoolIndex).Deallocate(items);
				if (memPoolIndex != pvGetMemPoolIndex(maxCount))
					memPoolIndex = pvGetMemPoolIndex(1);
				pvSet(nullptr, memPoolIndex, 0);
				return nullptr;
			}
			else
			{
				size_t index = iter - items;
				MOMO_ASSERT(index < count);
				itemReplacer(items[count - 1], items[index]);
				--mState;
				return items + index;
			}
		}

	private:
		void pvSet(Item* items, size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			*&mItemPtrBuffer = items;
			mState = (uint8_t)((memPoolIndex << 4) | count);
		}

		static size_t pvGetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(0 < count && count <= maxCount);
			if (Params::skipFirstMemPool && count == 1)
				return 2;
			return count;
		}

		size_t pvGetMemPoolIndex() const MOMO_NOEXCEPT
		{
			return (size_t)(mState >> 4);
		}

		size_t pvGetCount() const MOMO_NOEXCEPT
		{
			return (size_t)(mState & 15);
		}

		Item* pvGetItems() const MOMO_NOEXCEPT
		{
			return *&mItemPtrBuffer;
		}

	private:
		ObjectBuffer<Item*, alignment> mItemPtrBuffer;
		uint8_t mState;
	};
}

template<size_t tMaxCount = 4,
	typename TMemPoolParams = MemPoolParams<>,
	size_t tAlignment = MOMO_ALIGNMENT_OF(void*)>
struct HashBucketLimP1 : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;
	static const size_t alignment = tAlignment;

	typedef TMemPoolParams MemPoolParams;

	template<typename ItemTraits>
	using Bucket = internal::BucketLimP1<ItemTraits, maxCount, MemPoolParams, alignment>;
};

} // namespace momo

/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/details/HashBucketLimP1.h

  namespace momo:
    class HashBucketLimP1

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../MemPool.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, size_t tMaxCount, typename TMemPoolParams>
	class BucketLimP1 : public BucketBase
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;

	public:
		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount < 16);

		static const bool isNothrowAddableIfNothrowCreatable = false;

		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef Item* Iterator;
		typedef ArrayBounds<Iterator> Bounds;

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

			~Params() = default;

			Params& operator=(const Params&) = delete;

			void Clear() noexcept
			{
				for (MemPool& memPool : mMemPools)
				{
					if (memPool.CanDeallocateAll())
						memPool.DeallocateAll();
				}
			}

			MemManager& GetMemManager() noexcept
			{
				return mMemPools[0].GetMemManager().GetBaseMemManager();
			}

			MemPool& GetMemPool(size_t memPoolIndex) noexcept
			{
				MOMO_ASSERT(memPoolIndex >= minMemPoolIndex);
				return mMemPools[memPoolIndex - minMemPoolIndex];
			}

		private:
			MemPools mMemPools;
		};

	public:
		explicit BucketLimP1() noexcept
		{
			pvSet(nullptr, pvGetMemPoolIndex(1), 0);
		}

		BucketLimP1(const BucketLimP1&) = delete;

		~BucketLimP1() noexcept
		{
			MOMO_ASSERT(pvGetItems() == nullptr);
		}

		BucketLimP1& operator=(const BucketLimP1&) = delete;

		Bounds GetBounds(Params& /*params*/) noexcept
		{
			return Bounds(pvGetItems(), pvGetCount());
		}

		template<bool first, typename Predicate>
		MOMO_FORCEINLINE Iterator Find(Params& /*params*/, const Predicate& pred, size_t /*hashCode*/)
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

		bool IsFull() const noexcept
		{
			return pvGetCount() == maxCount;
		}

		bool WasFull() const noexcept
		{
			return pvGetMemPoolIndex() == pvGetMemPoolIndex(maxCount);
		}

		void Clear(Params& params) noexcept
		{
			Item* items = pvGetItems();
			if (items != nullptr)
			{
				MemPool& memPool = params.GetMemPool(pvGetMemPoolIndex());
				if (!memPool.CanDeallocateAll())
					memPool.Deallocate(items);
			}
			pvSet(nullptr, pvGetMemPoolIndex(1), 0);
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& params, ItemCreator&& itemCreator, size_t /*hashCode*/,
			size_t /*logBucketCount*/, size_t /*probe*/)
		{
			Item* items = pvGetItems();
			if (items == nullptr)
			{
				size_t newCount = 1;
				size_t newMemPoolIndex = pvGetMemPoolIndex();
				Memory memory(params.GetMemPool(newMemPoolIndex));
				Item* newItems = memory.GetPointer();
				std::forward<ItemCreator>(itemCreator)(newItems);
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
						std::forward<ItemCreator>(itemCreator), newItems + count);
					params.GetMemPool(memPoolIndex).Deallocate(items);
					pvSet(memory.Extract(), newMemPoolIndex, newCount);
					return newItems + count;
				}
				else
				{
					std::forward<ItemCreator>(itemCreator)(items + count);
					++mState;
					return items + count;
				}
			}
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& params, Iterator iter, ItemReplacer&& itemReplacer)
		{
			size_t count = pvGetCount();
			MOMO_ASSERT(count > 0);
			Item* items = pvGetItems();
			if (count == 1)
			{
				MOMO_ASSERT(iter == items);
				std::forward<ItemReplacer>(itemReplacer)(*items, *items);
				size_t memPoolIndex = pvGetMemPoolIndex();
				params.GetMemPool(memPoolIndex).Deallocate(items);
				if (memPoolIndex != pvGetMemPoolIndex(maxCount))
					memPoolIndex = pvGetMemPoolIndex(1);
				pvSet(nullptr, memPoolIndex, 0);
				return nullptr;
			}
			else
			{
				MOMO_ASSERT(items <= iter && iter < items + count);
				std::forward<ItemReplacer>(itemReplacer)(items[count - 1], *iter);
				--mState;
				return iter;
			}
		}

	private:
		void pvSet(Item* items, size_t memPoolIndex, size_t count) noexcept
		{
			PtrCaster::ToBuffer(items, mItemPtrBuffer);
			mState = static_cast<uint8_t>((memPoolIndex << 4) | count);
		}

		static size_t pvGetMemPoolIndex(size_t count) noexcept
		{
			MOMO_ASSERT(0 < count && count <= maxCount);
			if (Params::skipFirstMemPool && count == 1)
				return 2;
			return count;
		}

		size_t pvGetMemPoolIndex() const noexcept
		{
			return size_t{mState} >> 4;
		}

		size_t pvGetCount() const noexcept
		{
			return size_t{mState} & 15;
		}

		Item* pvGetItems() const noexcept
		{
			return PtrCaster::template FromBuffer<Item>(mItemPtrBuffer);
		}

	private:
		char mItemPtrBuffer[sizeof(Item*)];
		uint8_t mState;
	};
}

template<size_t tMaxCount = 4,
	typename TMemPoolParams = MemPoolParams<>>
class HashBucketLimP1 : public internal::HashBucketBase
{
public:
	static const size_t maxCount = tMaxCount;

	typedef TMemPoolParams MemPoolParams;

	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = internal::BucketLimP1<ItemTraits, maxCount, MemPoolParams>;
};

} // namespace momo

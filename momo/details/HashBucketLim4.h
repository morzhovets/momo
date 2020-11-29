/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/details/HashBucketLim4.h

  namespace momo:
    class HashBucketLim4

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../MemPool.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, size_t tLogMaxCount, size_t tMemPoolBlockCount>
	class BucketLim4 : public BucketBase
	{
	protected:
		typedef TItemTraits ItemTraits;

		static const size_t logMaxCount = tLogMaxCount;
		MOMO_STATIC_ASSERT(0 < logMaxCount && logMaxCount <= 4);	//?

		static const size_t memPoolBlockCount = tMemPoolBlockCount;

	public:
		static const size_t maxCount = size_t{1} << logMaxCount;

		static const bool isNothrowAddableIfNothrowCreatable = false;

		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef Item* Iterator;
		typedef ArrayBounds<Iterator> Bounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef MemPoolUInt32<memPoolBlockCount, MemManagerPtr> MemPool;

		typedef BucketMemory<MemPool, uint32_t, MemPool::nullPtr> Memory;

		static const uint32_t stateNull = (uint32_t{1} << (32 - logMaxCount)) - 1;
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
							? (size_t{1} << (32 - logMaxCount)) - 2
							: (size_t{1} << (32 - logMaxCount)) / i;
						::new(static_cast<void*>(newMemPool)) MemPool(i * sizeof(Item),
							MemManagerPtr(memManager), maxTotalBlockCount);
					};
					mMemPools.AddBackNogrowCrt(memPoolCreator);
				}
			}

			Params(const Params&) = delete;

			~Params() = default;

			Params& operator=(const Params&) = delete;

			void Clear() noexcept
			{
			}

			MemManager& GetMemManager() noexcept
			{
				return mMemPools[0].GetMemManager().GetBaseMemManager();
			}

			MemPool& GetMemPool(size_t memPoolIndex) noexcept
			{
				MOMO_ASSERT(memPoolIndex > 0);
				return mMemPools[memPoolIndex - 1];
			}

		private:
			MemPools mMemPools;
		};

	public:
		explicit BucketLim4() noexcept
			: mPtrState(stateNull)
		{
		}

		BucketLim4(const BucketLim4&) = delete;

		~BucketLim4() noexcept
		{
			MOMO_ASSERT(pvIsEmpty());
		}

		BucketLim4& operator=(const BucketLim4&) = delete;

		Bounds GetBounds(Params& params) noexcept
		{
			if (pvIsEmpty())
				return Bounds();
			Data data = pvGetData();
			MemPool& memPool = params.GetMemPool(pvGetMemPoolIndex());
			Item* items = memPool.template GetRealPointer<Item>(data.pointer);
			return Bounds(items, data.count);
		}

		template<bool first, typename Predicate>
		MOMO_FORCEINLINE Iterator Find(Params& params, const Predicate& pred, size_t /*hashCode*/)
		{
			for (Item& item : GetBounds(params))
			{
				if (pred(item))
					return std::addressof(item);
			}
			return nullptr;
		}

		bool IsFull() const noexcept
		{
			if (pvIsEmpty())
				return false;
			return pvGetData().count == maxCount;
		}

		bool WasFull() const noexcept
		{
			if (mPtrState == stateNull)
				return false;
			if (mPtrState == stateNullWasFull)
				return true;
			return pvGetMemPoolIndex() == pvGetMemPoolIndex(maxCount);
		}

		void Clear(Params& params) noexcept
		{
			if (!pvIsEmpty())
			{
				uint32_t ptr = pvGetData().pointer;
				MemPool& memPool = params.GetMemPool(pvGetMemPoolIndex());
				memPool.Deallocate(ptr);
			}
			mPtrState = stateNull;
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& params, ItemCreator&& itemCreator, size_t /*hashCode*/,
			size_t /*logBucketCount*/, size_t /*probe*/)
		{
			if (pvIsEmpty())
			{
				size_t newCount = 1;
				size_t newMemPoolIndex =
					pvGetMemPoolIndex((mPtrState == stateNull) ? newCount : maxCount);
				MemPool& newMemPool = params.GetMemPool(newMemPoolIndex);
				Memory memory(newMemPool);
				Item* newItems = newMemPool.template GetRealPointer<Item>(memory.GetPointer());
				std::forward<ItemCreator>(itemCreator)(newItems);
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
						std::forward<ItemCreator>(itemCreator), newItems + count);
					memPool.Deallocate(ptr);
					pvSet(memory.Extract(), newMemPoolIndex, newCount);
					return newItems + count;
				}
				else
				{
					std::forward<ItemCreator>(itemCreator)(items + count);
					++mPtrState;
					return items + count;
				}
			}
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& params, Iterator iter, ItemReplacer&& itemReplacer)
		{
			MOMO_ASSERT(!pvIsEmpty());
			Data data = pvGetData();
			size_t count = data.count;
			uint32_t ptr = data.pointer;
			size_t memPoolIndex = pvGetMemPoolIndex();
			MemPool& memPool = params.GetMemPool(memPoolIndex);
			Item* items = memPool.template GetRealPointer<Item>(ptr);
			if (count == 1)
			{
				MOMO_ASSERT(iter == items);
				std::forward<ItemReplacer>(itemReplacer)(*items, *items);
				memPool.Deallocate(ptr);
				mPtrState = (memPoolIndex < pvGetMemPoolIndex(maxCount))
					? stateNull : stateNullWasFull;
				return nullptr;
			}
			else
			{
				MOMO_ASSERT(items <= iter && iter < items + count);
				std::forward<ItemReplacer>(itemReplacer)(items[count - 1], *iter);
				--mPtrState;
				return iter;
			}
		}

	private:
		bool pvIsEmpty() const noexcept
		{
			return mPtrState == stateNull || mPtrState == stateNullWasFull;
		}

		void pvSet(uint32_t ptr, size_t memPoolIndex, size_t count)
		{
			mPtrState = static_cast<uint32_t>(((memPoolIndex - 1) << (32 - logMaxCount))
				+ size_t{ptr} * memPoolIndex + count - 1);
		}

		static size_t pvGetMemPoolIndex(size_t count) noexcept
		{
			MOMO_ASSERT(0 < count && count <= maxCount);
			return count;
		}

		size_t pvGetMemPoolIndex() const noexcept
		{
			MOMO_ASSERT(!pvIsEmpty());
			return size_t{mPtrState >> (32 - logMaxCount)} + 1;
		}

		Data pvGetData() const noexcept
		{
			UIntMath<uint32_t>::DivResult divRes = UIntMath<uint32_t>::DivBySmall(
				mPtrState & stateNull, static_cast<uint32_t>(pvGetMemPoolIndex()));
			Data data;
			data.pointer = divRes.quotient;
			data.count = size_t{divRes.remainder} + 1;
			return data;
		}

	private:
		uint32_t mPtrState;
	};
}

template<size_t tLogMaxCount = 2,
	size_t tMemPoolBlockCount = MemPoolConst::defaultBlockCount>
class HashBucketLim4 : public internal::HashBucketBase
{
public:
	static const size_t logMaxCount = tLogMaxCount;
	static const size_t memPoolBlockCount = tMemPoolBlockCount;

	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = internal::BucketLim4<ItemTraits, logMaxCount, memPoolBlockCount>;
};

} // namespace momo

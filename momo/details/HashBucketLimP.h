/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/details/HashBucketLimP.h

  namespace momo:
    class HashBucketLimP

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../MemPool.h"

namespace momo
{

namespace internal
{
	template<size_t maxCount>
	concept conceptBucketLimPMaxCount = (0 < maxCount && maxCount < 16);

	template<typename TItemTraits, size_t tMaxCount,
		conceptMemPoolParamsBlockSizeAlignment TMemPoolParams, bool tUsePtrState>
	class BucketLimP;

	template<typename TItemTraits, size_t tMaxCount,
		conceptMemPoolParamsBlockSizeAlignment TMemPoolParams>
	requires conceptBucketLimPMaxCount<tMaxCount>
	class BucketLimP<TItemTraits, tMaxCount, TMemPoolParams, false> : public BucketBase
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;

		static const bool usePtrState = false;

	public:
		static const size_t maxCount = tMaxCount;

		static const bool isNothrowAddableIfNothrowCreatable = false;

		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef Item* Iterator;
		typedef ArrayBounds<Iterator> Bounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef momo::MemPool<MemPoolParams, MemManagerPtr, NestedMemPoolSettings> MemPool;

		typedef BucketMemory<MemPool, uint8_t*> Memory;

		static const uintptr_t ptrNull = UIntConst::nullPtr;
		static const uintptr_t ptrNullWasFull = UIntConst::invalidPtr;

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

			~Params() noexcept = default;

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
				MOMO_ASSERT(memPoolIndex > 0);
				return mMemPools[memPoolIndex - 1];
			}

		private:
			MemPools mMemPools;
		};

	public:
		explicit BucketLimP() noexcept
			: mPtr(ptrNull)
		{
		}

		BucketLimP(const BucketLimP&) = delete;

		~BucketLimP() noexcept = default;

		BucketLimP& operator=(const BucketLimP&) = delete;

		Bounds GetBounds(Params& /*params*/) noexcept
		{
			if (pvIsEmpty())
				return Bounds();
			return Bounds(pvGetItems(), pvGetCount());
		}

		template<bool first, typename Predicate>
		requires std::predicate<Predicate, const Item&>
		MOMO_FORCEINLINE Iterator Find(Params& /*params*/, Predicate pred, size_t /*hashCode*/)
		{
			if (pvIsEmpty())
				return nullptr;
			size_t count = pvGetCount();
			Item* items = pvGetItems();
			for (size_t i = 0; i < count; ++i)
			{
				if (pred(std::as_const(items[i])))
					return items + i;
			}
			return nullptr;
		}

		bool IsFull() const noexcept
		{
			if (pvIsEmpty())
				return false;
			return pvGetCount() == maxCount;
		}

		bool WasFull() const noexcept
		{
			if (mPtr == ptrNull)
				return false;
			if (mPtr == ptrNullWasFull)
				return true;
			return pvGetMemPoolIndex() == pvGetMemPoolIndex(maxCount);
		}

		void Clear(Params& params) noexcept
		{
			if (!pvIsEmpty())
			{
				MemPool& memPool = params.GetMemPool(pvGetMemPoolIndex());
				if (!memPool.CanDeallocateAll())
					memPool.Deallocate(pvGetPtr());
			}
			mPtr = ptrNull;
		}

		template<conceptCreator<Item> ItemCreator>
		Iterator AddCrt(Params& params, ItemCreator itemCreator, size_t /*hashCode*/,
			size_t /*logBucketCount*/, size_t /*probe*/)
		{
			if (pvIsEmpty())
			{
				size_t newCount = 1;
				size_t newMemPoolIndex = pvGetMemPoolIndex((mPtr == ptrNull) ? newCount : maxCount);
				Memory memory(params.GetMemPool(newMemPoolIndex));
				Item* newItems = pvGetItems(memory.GetPointer());
				std::move(itemCreator)(newItems);
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
						count, std::move(itemCreator), newItems + count);
					params.GetMemPool(memPoolIndex).Deallocate(pvGetPtr());
					pvSet(memory.Extract(), newMemPoolIndex, newCount);
					return newItems + count;
				}
				else
				{
					Item* items = pvGetItems();
					std::move(itemCreator)(items + count);
					++*pvGetPtr();
					return items + count;
				}
			}
		}

		template<conceptReplacer<Item> ItemReplacer>
		Iterator Remove(Params& params, Iterator iter, ItemReplacer itemReplacer)
		{
			size_t count = pvGetCount();
			MOMO_ASSERT(count > 0);
			Item* items = pvGetItems();
			if (count == 1)
			{
				MOMO_ASSERT(iter == items);
				std::move(itemReplacer)(*items, *items);
				size_t memPoolIndex = pvGetMemPoolIndex();
				params.GetMemPool(memPoolIndex).Deallocate(pvGetPtr());
				mPtr = (memPoolIndex < pvGetMemPoolIndex(maxCount)) ? ptrNull : ptrNullWasFull;
				return nullptr;
			}
			else
			{
				MOMO_ASSERT(items <= iter && iter < items + count);
				std::move(itemReplacer)(items[count - 1], *iter);
				--*pvGetPtr();
				return iter;
			}
		}

	private:
		bool pvIsEmpty() const noexcept
		{
			return mPtr == ptrNull || mPtr == ptrNullWasFull;
		}

		uint8_t* pvGetPtr() const noexcept
		{
			MOMO_ASSERT(!pvIsEmpty());
			return PtrCaster::FromUInt<uint8_t>(mPtr);
		}

		void pvSet(uint8_t* ptr, size_t memPoolIndex, size_t count) noexcept
		{
			MOMO_ASSERT(ptr != nullptr);
			mPtr = PtrCaster::ToUInt(ptr);
			*ptr = static_cast<uint8_t>((memPoolIndex << 4) | count);
		}

		static size_t pvGetMemPoolIndex(size_t count) noexcept
		{
			MOMO_ASSERT(0 < count && count <= maxCount);
			return count;
		}

		size_t pvGetMemPoolIndex() const noexcept
		{
			return size_t{*pvGetPtr()} >> 4;
		}

		size_t pvGetCount() const noexcept
		{
			return size_t{*pvGetPtr()} & 15;
		}

		Item* pvGetItems() const noexcept
		{
			return pvGetItems(pvGetPtr());
		}

		static Item* pvGetItems(uint8_t* ptr) noexcept
		{
			return PtrCaster::Shift<Item>(ptr, ItemTraits::alignment);
		}

	private:
		uintptr_t mPtr;
	};

	template<typename TItemTraits, size_t tMaxCount,
		conceptMemPoolParamsBlockSizeAlignment TMemPoolParams>
	requires conceptBucketLimPMaxCount<tMaxCount>
	class BucketLimP<TItemTraits, tMaxCount, TMemPoolParams, true> : public BucketBase
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;

		static const bool usePtrState = true;

	public:
		static const size_t maxCount = tMaxCount;

		static const bool isNothrowAddableIfNothrowCreatable = false;

		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef Item* Iterator;
		typedef ArrayBounds<Iterator> Bounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef momo::MemPool<MemPoolParams, MemManagerPtr, NestedMemPoolSettings> MemPool;

		typedef BucketMemory<MemPool, Item*> Memory;

		static const size_t minItemAlignment = (maxCount <= 1) ? 1 : (maxCount <= 2) ? 2
			: (maxCount <= 4) ? 4 : (maxCount <= 8) ? 8 : 16;
		static const size_t itemAlignment = std::minmax(ItemTraits::alignment, minItemAlignment).second;

		static const bool skipOddMemPools = (maxCount > 1 && sizeof(Item) <= itemAlignment);	//?
		static const uintptr_t modMemPoolIndex =
			uintptr_t{minItemAlignment} / (skipOddMemPools ? 2 : 1);

		static const uintptr_t stateNull = UIntConst::nullPtr;
		static const uintptr_t stateNullWasFull = UIntConst::invalidPtr;

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
					size_t blockAlignment = UIntMath<>::Ceil(i * size_t{modMemPoolIndex},
						itemAlignment);
					mMemPools.AddBackNogrow(MemPool(MemPoolParams(i * sizeof(Item), blockAlignment),
						MemManagerPtr(memManager)));
				}
			}

			Params(const Params&) = delete;

			~Params() noexcept = default;

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
				MOMO_ASSERT(memPoolIndex > 0);
				return mMemPools[(memPoolIndex - 1) / (skipOddMemPools ? 2 : 1)];
			}

		private:
			MemPools mMemPools;
		};

	public:
		explicit BucketLimP() noexcept
			: mPtrState(stateNull)
		{
		}

		BucketLimP(const BucketLimP&) = delete;

		~BucketLimP() noexcept = default;

		BucketLimP& operator=(const BucketLimP&) = delete;

		Bounds GetBounds(Params& /*params*/) noexcept
		{
			if (pvIsEmpty())
				return Bounds();
			return pvGetBounds();
		}

		template<bool first, typename Predicate>
		requires std::predicate<Predicate, const Item&>
		MOMO_FORCEINLINE Iterator Find(Params& /*params*/, Predicate pred, size_t /*hashCode*/)
		{
			if (pvIsEmpty())
				return nullptr;
			for (Item& item : pvGetBounds())
			{
				if (pred(std::as_const(item)))
					return std::addressof(item);
			}
			return nullptr;
		}

		bool IsFull() const noexcept
		{
			if (pvIsEmpty())
				return false;
			return pvGetBounds().GetCount() == maxCount;
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
				MemPool& memPool = params.GetMemPool(pvGetMemPoolIndex());
				if (!memPool.CanDeallocateAll())
					memPool.Deallocate(pvGetBounds().GetBegin());
			}
			mPtrState = stateNull;
		}

		template<conceptCreator<Item> ItemCreator>
		Iterator AddCrt(Params& params, ItemCreator itemCreator, size_t /*hashCode*/,
			size_t /*logBucketCount*/, size_t /*probe*/)
		{
			if (pvIsEmpty())
			{
				size_t newCount = 1;
				size_t newMemPoolIndex =
					pvGetMemPoolIndex((mPtrState == stateNull) ? newCount : maxCount);
				Memory memory(params.GetMemPool(newMemPoolIndex));
				Item* newItems = memory.GetPointer();
				std::move(itemCreator)(newItems);
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
						std::move(itemCreator), newItems + count);
					params.GetMemPool(memPoolIndex).Deallocate(items);
					pvSet(memory.Extract(), newMemPoolIndex, newCount);
					return newItems + count;
				}
				else
				{
					std::move(itemCreator)(items + count);
					mPtrState += modMemPoolIndex;
					return items + count;
				}
			}
		}

		template<conceptReplacer<Item> ItemReplacer>
		Iterator Remove(Params& params, Iterator iter, ItemReplacer itemReplacer)
		{
			MOMO_ASSERT(!pvIsEmpty());
			Bounds bounds = pvGetBounds();
			Item* items = bounds.GetBegin();
			size_t count = bounds.GetCount();
			if (count == 1)
			{
				MOMO_ASSERT(iter == items);
				std::move(itemReplacer)(*items, *items);
				size_t memPoolIndex = pvGetMemPoolIndex();
				params.GetMemPool(memPoolIndex).Deallocate(items);
				mPtrState = (memPoolIndex < pvGetMemPoolIndex(maxCount))
					? stateNull : stateNullWasFull;
				return nullptr;
			}
			else
			{
				MOMO_ASSERT(items <= iter && iter < items + count);
				std::move(itemReplacer)(items[count - 1], *iter);
				mPtrState -= modMemPoolIndex;
				return iter;
			}
		}

	private:
		bool pvIsEmpty() const noexcept
		{
			return mPtrState == stateNull || mPtrState == stateNullWasFull;
		}

		void pvSet(Item* items, size_t memPoolIndex, size_t count) noexcept
		{
			mPtrState = PtrCaster::ToUInt(items) + static_cast<uintptr_t>(count - 1) * modMemPoolIndex
				+ uintptr_t{memPoolIndex} / (skipOddMemPools ? 2 : 1) - 1;
		}

		static size_t pvGetMemPoolIndex(size_t count) noexcept
		{
			MOMO_ASSERT(0 < count && count <= maxCount);
			return count + (skipOddMemPools ? count % 2 : 0);
		}

		size_t pvGetMemPoolIndex() const noexcept
		{
			MOMO_ASSERT(!pvIsEmpty());
			return static_cast<size_t>((mPtrState % modMemPoolIndex) + 1) * (skipOddMemPools ? 2 : 1);
		}

		Bounds pvGetBounds() const noexcept
		{
			uintptr_t memPoolIndex = uintptr_t{pvGetMemPoolIndex()};
			uintptr_t ptrCount = mPtrState / modMemPoolIndex;
			uintptr_t mod = UIntMath<uintptr_t>::Ceil(memPoolIndex,
				uintptr_t{itemAlignment} / modMemPoolIndex);
			uintptr_t count1 = UIntMath<uintptr_t>::DivBySmall(ptrCount, mod).remainder;
			MOMO_ASSERT(count1 < memPoolIndex);
			Item* items = PtrCaster::FromUInt<Item>((ptrCount - count1) * modMemPoolIndex);
			return Bounds(items, static_cast<size_t>(count1) + 1);
		}

	private:
		uintptr_t mPtrState;
	};
}

template<size_t tMaxCount = sizeof(void*),
	internal::conceptMemPoolParamsBlockSizeAlignment TMemPoolParams = MemPoolParams<>,
	bool tUsePtrState = true>
requires internal::conceptBucketLimPMaxCount<tMaxCount>
class HashBucketLimP : public internal::HashBucketBase
{
public:
	static const size_t maxCount = tMaxCount;
	static const bool usePtrState = tUsePtrState;

	typedef TMemPoolParams MemPoolParams;

public:	// clang
	template<typename ItemTraits>
	static constexpr bool CorrectUsePtrState() noexcept
	{
		size_t size = sizeof(typename ItemTraits::Item);
		size_t alignment = ItemTraits::alignment;
		return usePtrState && ((maxCount <= 1)
			|| (maxCount <= 2 && size % 2 == 0 && (size > 2 || alignment == 2))
			|| (maxCount <= 4 && size % 4 == 0 && (size > 4 || alignment == 4))
			|| (maxCount <= 8 && size % 8 == 0 && (size > 8 || alignment == 8))
			|| (maxCount <= 16 && size % 16 == 0 && (size > 16 || alignment == 16)));
	}

public:
	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = internal::BucketLimP<ItemTraits, maxCount, MemPoolParams,
		CorrectUsePtrState<ItemTraits>()>;
};

} // namespace momo

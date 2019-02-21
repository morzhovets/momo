/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/details/HashBucketLimP4.h

  namespace momo:
    class HashBucketLimP4

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../MemPool.h"

namespace momo
{

namespace internal
{
	template<typename TItem, uint8_t tMaskState, size_t tBitCount,
		typename = void>
	class BucketLimP4PtrState;

	template<typename TItem, uint8_t tMaskState, size_t tBitCount>
	class BucketLimP4PtrState<TItem, tMaskState, tBitCount,
		EnableIf<(tBitCount <= 32), void>>
	{
	public:
		typedef TItem Item;

		static const uint8_t maskState = tMaskState;
		static const size_t bitCount = 32;

		MOMO_STATIC_ASSERT(((uint8_t)UIntPtrConst::null & maskState) == (uint8_t)0);

	public:
		void Set(Item* ptr, uint8_t state) noexcept
		{
			MOMO_ASSERT(state <= maskState);
			mPtrState = (uint32_t)reinterpret_cast<uintptr_t>(ptr);
			MOMO_ASSERT(((uint8_t)mPtrState & maskState) == (uint8_t)0);
			mPtrState |= (uint32_t)state;
		}

		Item* GetPointer() const noexcept
		{
			uintptr_t intPtr = (uintptr_t)(mPtrState & ~(uint32_t)maskState);
			return reinterpret_cast<Item*>(intPtr);
		}

		uint8_t GetState() const noexcept
		{
			return (uint8_t)mPtrState & maskState;
		}

	private:
		uint32_t mPtrState;
	};

	template<typename TItem, uint8_t tMaskState, size_t tBitCount>
	class BucketLimP4PtrState<TItem, tMaskState, tBitCount,
		EnableIf<(32 < tBitCount && tBitCount <= 48), void>>
	{
	public:
		typedef TItem Item;

		static const uint8_t maskState = tMaskState;
		static const size_t bitCount = 48;

		MOMO_STATIC_ASSERT(((uint8_t)UIntPtrConst::null & maskState) == (uint8_t)0);

	public:
		void Set(Item* ptr, uint8_t state) noexcept
		{
			MOMO_ASSERT(state <= maskState);
			uint64_t intPtr = reinterpret_cast<uint64_t>(ptr);
			MOMO_ASSERT(((uint8_t)intPtr & maskState) == (uint8_t)0);
			mPtrState[0] = (uint16_t)intPtr | (uint16_t)state;
			mPtrState[1] = (uint16_t)(intPtr >> 16);
			mPtrState[2] = (uint16_t)(intPtr >> 32);
		}

		Item* GetPointer() const noexcept
		{
			uint64_t intPtr = ((uint64_t)mPtrState[2] << 32) | ((uint64_t)mPtrState[1] << 16)
				| (uint64_t)(mPtrState[0] & ~(uint16_t)maskState);
			return reinterpret_cast<Item*>(intPtr);
		}

		uint8_t GetState() const noexcept
		{
			return (uint8_t)mPtrState[0] & maskState;
		}

	private:
		uint16_t mPtrState[3];
	};

	template<typename TItem, uint8_t tMaskState, size_t tBitCount>
	class BucketLimP4PtrState<TItem, tMaskState, tBitCount,
		EnableIf<(48 < tBitCount && tBitCount <= 64), void>>
	{
	public:
		typedef TItem Item;

		static const uint8_t maskState = tMaskState;
		static const size_t bitCount = 64;

		MOMO_STATIC_ASSERT(((uint8_t)UIntPtrConst::null & maskState) == (uint8_t)0);

	public:
		void Set(Item* ptr, uint8_t state) noexcept
		{
			MOMO_ASSERT(state <= maskState);
			uint64_t intPtr = reinterpret_cast<uint64_t>(ptr);
			MOMO_ASSERT(((uint8_t)intPtr & maskState) == (uint8_t)0);
			mPtrState[0] = (uint32_t)intPtr | (uint32_t)state;
			mPtrState[1] = (uint32_t)(intPtr >> 32);
		}

		Item* GetPointer() const noexcept
		{
			uint64_t intPtr = ((uint64_t)mPtrState[1] << 32)
				| (uint64_t)(mPtrState[0] & ~(uint32_t)maskState);
			return reinterpret_cast<Item*>(intPtr);
		}

		uint8_t GetState() const noexcept
		{
			return (uint8_t)mPtrState[0] & maskState;
		}

	private:
		uint32_t mPtrState[2];
	};

	template<typename TItemTraits, size_t tMaxCount, typename TMemPoolParams,
		bool tUseHashCodePartGetter>
	class BucketLimP4 : public BucketBase
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;

		static const bool useHashCodePartGetter = tUseHashCodePartGetter;

	public:
		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount <= 4);

		static const bool isNothrowAddableIfNothrowCreatable = false;

		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef Item* Iterator;
		typedef ArrayBounds<Iterator> Bounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		static const size_t itemAlignment = (!useHashCodePartGetter || ItemTraits::alignment > 4)
			? ItemTraits::alignment : 4;

		static const size_t minMemPoolIndex =
			(maxCount > 1 && sizeof(Item) <= itemAlignment) ? 2 : 1;

		template<size_t memPoolIndex>
		using MemPoolParamsStatic = momo::MemPoolParamsStatic<memPoolIndex * sizeof(Item),
			itemAlignment, MemPoolParams::blockCount,
			(minMemPoolIndex <= memPoolIndex && memPoolIndex <= maxCount)
				? MemPoolParams::cachedFreeBlockCount : 0>;

		template<size_t memPoolIndex>
		using MemPool = momo::MemPool<MemPoolParamsStatic<memPoolIndex>, MemManagerPtr,
			NestedMemPoolSettings>;

		template<size_t memPoolIndex>
		using Memory = BucketMemory<MemPool<memPoolIndex>, Item*>;

		typedef BucketLimP4PtrState<Item, useHashCodePartGetter ? 3 : 0,
			MemManagerProxy<MemManager>::ptrUsefulBitCount> PtrState;

		MOMO_STATIC_ASSERT(PtrState::bitCount % 8 == 0);
		static const size_t hashCount = 4 +
			(useHashCodePartGetter ? sizeof(void*) - PtrState::bitCount / 8 : 0);

		static const uint8_t maskEmpty = 128;
		static const uint8_t emptyHashProbe = 255;

		static const size_t logBucketCountStep = 8;
		static const size_t logBucketCountAddend = 6;
		static const size_t hashCodeShift = sizeof(size_t) * 8 - 7;

	public:
		class Params
		{
		private:
			typedef std::tuple<MemPool<1>, MemPool<2>, MemPool<3>, MemPool<4>> MemPools;

		public:
			explicit Params(MemManager& memManager)
				: mMemPools(MemPool<1>(MemManagerPtr(memManager)),
					MemPool<2>(MemManagerPtr(memManager)),
					MemPool<3>(MemManagerPtr(memManager)),
					MemPool<4>(MemManagerPtr(memManager)))
			{
			}

			Params(const Params&) = delete;

			~Params() noexcept
			{
			}

			Params& operator=(const Params&) = delete;

			MemManager& GetMemManager() noexcept
			{
				return std::get<0>(mMemPools).GetMemManager().GetBaseMemManager();
			}

			template<size_t memPoolIndex>
			MemPool<memPoolIndex>& GetMemPool() noexcept
			{
				return std::get<memPoolIndex - 1>(mMemPools);
			}

		private:
			MemPools mMemPools;
		};

	public:
		explicit BucketLimP4() noexcept
		{
			pvSetEmpty(minMemPoolIndex);
		}

		BucketLimP4(const BucketLimP4&) = delete;

		~BucketLimP4() noexcept
		{
			MOMO_ASSERT(mPtrState.GetPointer() == nullptr);
		}

		BucketLimP4& operator=(const BucketLimP4&) = delete;

		Bounds GetBounds(Params& /*params*/) noexcept
		{
			return Bounds(mPtrState.GetPointer(), pvGetCount());
		}

		template<typename Predicate>
		Iterator Find(Params& /*params*/, const Predicate& pred, size_t hashCode)
		{
			uint8_t shortHash = pvCalcShortHash(hashCode);
			for (size_t i = 0; i < maxCount; ++i)
			{
				if (mShortHashes[i] == shortHash)
				{
					Item* items = mPtrState.GetPointer();
					if (pred(items[i]))
						return items + i;
				}
			}
			return nullptr;
		}

		bool IsFull() const noexcept
		{
			return mShortHashes[maxCount - 1] < maskEmpty;
		}

		bool WasFull() const noexcept
		{
			return pvGetMemPoolIndex() == maxCount;
		}

		void Clear(Params& params) noexcept
		{
			Item* items = mPtrState.GetPointer();
			if (items != nullptr)
			{
				ItemTraits::Destroy(params.GetMemManager(), items, pvGetCount());
				pvDeallocate(params, pvGetMemPoolIndex(), items);
			}
			pvSetEmpty(minMemPoolIndex);
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& params, ItemCreator&& itemCreator, size_t hashCode,
			size_t logBucketCount, size_t probe)
		{
			Item* items = mPtrState.GetPointer();
			size_t memPoolIndex = pvGetMemPoolIndex();
			if (items == nullptr)
			{
				MOMO_ASSERT(memPoolIndex == minMemPoolIndex || memPoolIndex == maxCount);
				pvSetHashProbe(0, hashCode, logBucketCount, probe);
				if (memPoolIndex == minMemPoolIndex)
					return pvAdd0<minMemPoolIndex>(params, std::forward<ItemCreator>(itemCreator), hashCode);
				else
					return pvAdd0<maxCount>(params, std::forward<ItemCreator>(itemCreator), hashCode);
			}
			else
			{
				size_t count = pvGetCount();
				MOMO_ASSERT(0 < count && count <= memPoolIndex);
				MOMO_ASSERT(count < maxCount);
				if (count == memPoolIndex)
				{
					switch (memPoolIndex)
					{
					case 1:
						pvSetHashProbe(1, hashCode, logBucketCount, probe);
						return pvAdd<1>(params, std::forward<ItemCreator>(itemCreator),
							hashCode, items);
					case 2:
						pvSetHashProbe(2, hashCode, logBucketCount, probe);
						return pvAdd<2>(params, std::forward<ItemCreator>(itemCreator),
							hashCode, items);
					default:
						MOMO_ASSERT(memPoolIndex == 3);
						pvSetHashProbe(3, hashCode, logBucketCount, probe);
						return pvAdd<3>(params, std::forward<ItemCreator>(itemCreator),
							hashCode, items);
					}
				}
				else
				{
					pvSetHashProbe(count, hashCode, logBucketCount, probe);
					std::forward<ItemCreator>(itemCreator)(items + count);
					mShortHashes[count] = pvCalcShortHash(hashCode);
					pvSetPtrState(items, memPoolIndex);
					return items + count;
				}
			}
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& params, Iterator iter, ItemReplacer&& itemReplacer)
		{
			Item* items = mPtrState.GetPointer();
			MOMO_ASSERT(items != nullptr);
			size_t count = pvGetCount();
			size_t memPoolIndex = pvGetMemPoolIndex();
			if (count == 1)
			{
				MOMO_ASSERT(iter == items);
				std::forward<ItemReplacer>(itemReplacer)(*items, *items);
				pvDeallocate(params, memPoolIndex, items);
				if (memPoolIndex != maxCount)
					memPoolIndex = minMemPoolIndex;
				pvSetEmpty(memPoolIndex);
				return nullptr;
			}
			else
			{
				size_t index = iter - items;
				MOMO_ASSERT(index < count);
				std::forward<ItemReplacer>(itemReplacer)(items[count - 1], *iter);
				mShortHashes[index] = mShortHashes[count - 1];
				mShortHashes[count - 1] = emptyHashProbe;
				if (useHashCodePartGetter && hashCount - 1 - index >= count)
				{
					mShortHashes[hashCount - 1 - index] = (hashCount - count >= count)
						? mShortHashes[hashCount - count] : emptyHashProbe;
				}
				pvSetPtrState(items, memPoolIndex);
				return iter;
			}
		}

		template<typename HashCodeFullGetter>
		size_t GetHashCodePart(const HashCodeFullGetter& hashCodeFullGetter, Iterator iter,
			size_t bucketIndex, size_t logBucketCount, size_t newLogBucketCount)
		{
			if (!useHashCodePartGetter)
				return hashCodeFullGetter();
			Item* items = mPtrState.GetPointer();
			size_t index = iter - items;
			size_t hashProbe = (size_t)mShortHashes[hashCount - 1 - index];
			bool useFullGetter = ((uint8_t)(hashProbe + 1) <= maskEmpty ||
				(logBucketCount + logBucketCountAddend) / logBucketCountStep
				!= (newLogBucketCount + logBucketCountAddend) / logBucketCountStep);
			if (useFullGetter)
				return hashCodeFullGetter();
			size_t probeShift = pvGetProbeShift(logBucketCount);
			size_t probe = hashProbe & (((size_t)1 << probeShift) - 1);
			size_t bucketCount = (size_t)1 << logBucketCount;
			return ((bucketIndex + bucketCount - probe) & (bucketCount - 1))
				| (((hashProbe - (size_t)maskEmpty) >> probeShift) << logBucketCount)
				| ((size_t)mShortHashes[index] << hashCodeShift);
		}

		static size_t GetNextBucketIndex(size_t bucketIndex, size_t /*hashCode*/,
			size_t bucketCount, size_t /*probe*/) noexcept
		{
			return (bucketIndex + 1) & (bucketCount - 1);
		}

	private:
		void pvSetEmpty(size_t memPoolIndex) noexcept
		{
			std::fill_n(mShortHashes, hashCount, (uint8_t)emptyHashProbe);
			pvSetPtrState(nullptr, memPoolIndex);
		}

		void pvSetPtrState(Item* items, size_t memPoolIndex) noexcept
		{
			uint8_t memPoolIndex1 = (uint8_t)memPoolIndex - 1;
			mPtrState.Set(items, useHashCodePartGetter ? memPoolIndex1 : 0);
			if (!useHashCodePartGetter && (maxCount < 4 || !IsFull()))
				mShortHashes[3] = maskEmpty + memPoolIndex1;
		}

		size_t pvGetMemPoolIndex() const noexcept
		{
			if (useHashCodePartGetter)
				return (size_t)mPtrState.GetState() + 1;
			else if (maxCount == 4 && IsFull())
				return 4;
			else
				return (size_t)(mShortHashes[3] & 3) + 1;
		}

		size_t pvGetCount() const noexcept
		{
			return (mShortHashes[1] >= maskEmpty) ? ((mShortHashes[0] < maskEmpty) ? 1 : 0)
				: 2 + ((mShortHashes[2] < maskEmpty) ? 1 : 0) + ((mShortHashes[3] < maskEmpty) ? 1 : 0);
		}

		static uint8_t pvCalcShortHash(size_t hashCode) noexcept
		{
			return (uint8_t)(hashCode >> hashCodeShift);
		}

		static size_t pvGetProbeShift(size_t logBucketCount) noexcept
		{
			return (logBucketCount + logBucketCountAddend) % logBucketCountStep;
		}

		void pvSetHashProbe(size_t index, size_t hashCode, size_t logBucketCount,
			size_t probe) noexcept
		{
			if (!useHashCodePartGetter || hashCount - 1 - index <= index)
				return;
			size_t probeShift = pvGetProbeShift(logBucketCount);
			mShortHashes[hashCount - 1 - index] = (probe < (size_t)1 << probeShift)
				? maskEmpty | (uint8_t)((hashCode >> logBucketCount) << probeShift) | (uint8_t)probe
				: emptyHashProbe;
		}

		template<size_t memPoolIndex, typename ItemCreator>
		Item* pvAdd0(Params& params, ItemCreator&& itemCreator, size_t hashCode)
		{
			Memory<memPoolIndex> memory(params.template GetMemPool<memPoolIndex>());
			Item* items = memory.GetPointer();
			std::forward<ItemCreator>(itemCreator)(items);
			mShortHashes[0] = pvCalcShortHash(hashCode);
			pvSetPtrState(memory.Extract(), memPoolIndex);
			return items;
		}

		template<size_t memPoolIndex, typename ItemCreator>
		Item* pvAdd(Params& params, ItemCreator&& itemCreator, size_t hashCode, Item* items)
		{
			static const size_t newMemPoolIndex = memPoolIndex + 1;
			size_t count = memPoolIndex;
			Memory<newMemPoolIndex> memory(params.template GetMemPool<newMemPoolIndex>());
			Item* newItems = memory.GetPointer();
			ItemTraits::RelocateCreate(params.GetMemManager(), items, newItems, count,
				std::forward<ItemCreator>(itemCreator), newItems + count);
			params.template GetMemPool<memPoolIndex>().Deallocate(items);
			mShortHashes[count] = pvCalcShortHash(hashCode);
			pvSetPtrState(memory.Extract(), newMemPoolIndex);
			return newItems + count;
		}

		void pvDeallocate(Params& params, size_t memPoolIndex, Item* items) noexcept
		{
			switch (memPoolIndex)
			{
			case 1:
				params.template GetMemPool<1>().Deallocate(items);
				break;
			case 2:
				params.template GetMemPool<2>().Deallocate(items);
				break;
			case 3:
				params.template GetMemPool<3>().Deallocate(items);
				break;
			case 4:
				params.template GetMemPool<4>().Deallocate(items);
				break;
			default:
				MOMO_ASSERT(false);
			}
		}

	private:
		PtrState mPtrState;
		uint8_t mShortHashes[hashCount];
	};
}

template<size_t tMaxCount = 4,
	typename TMemPoolParams = MemPoolParams<>>
class HashBucketLimP4 : public internal::HashBucketBase
{
public:
	static const size_t maxCount = tMaxCount;

	typedef TMemPoolParams MemPoolParams;

	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = internal::BucketLimP4<ItemTraits, maxCount, MemPoolParams,
		useHashCodePartGetter && sizeof(typename ItemTraits::Item) >= 4>;
};

} // namespace momo

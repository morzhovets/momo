/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketLimP4.h

  namespace momo:
    struct HashBucketLimP4

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
		typename std::enable_if<(tBitCount <= 32)>::type>
	{
	public:
		typedef TItem Item;

		static const uint8_t maskState = tMaskState;
		static const size_t bitCount = 32;

	public:
		void Set(Item* ptr, uint8_t state) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(state <= maskState);
			mPtrState = (uint32_t)reinterpret_cast<uintptr_t>(ptr);
			MOMO_ASSERT(((uint8_t)mPtrState & maskState) == 0);
			mPtrState |= (uint32_t)state;
		}

		Item* GetPointer() const MOMO_NOEXCEPT
		{
			uintptr_t intPtr = (uintptr_t)(mPtrState & ~(uint32_t)maskState);
			return reinterpret_cast<Item*>(intPtr);
		}

		uint8_t GetState() const MOMO_NOEXCEPT
		{
			return (uint8_t)mPtrState & maskState;
		}

	private:
		uint32_t mPtrState;
	};

	template<typename TItem, uint8_t tMaskState, size_t tBitCount>
	class BucketLimP4PtrState<TItem, tMaskState, tBitCount,
		typename std::enable_if<(32 < tBitCount && tBitCount <= 48)>::type>
	{
	public:
		typedef TItem Item;

		static const uint8_t maskState = tMaskState;
		static const size_t bitCount = 48;

	public:
		void Set(Item* ptr, uint8_t state) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(state <= maskState);
			uint64_t intPtr = reinterpret_cast<uint64_t>(ptr);
			MOMO_ASSERT(((uint8_t)intPtr & maskState) == 0);
			mPtrState[0] = (uint16_t)intPtr | (uint16_t)state;
			mPtrState[1] = (uint16_t)(intPtr >> 16);
			mPtrState[2] = (uint16_t)(intPtr >> 32);
		}

		Item* GetPointer() const MOMO_NOEXCEPT
		{
			uint64_t intPtr = ((uint64_t)mPtrState[2] << 32) | ((uint64_t)mPtrState[1] << 16)
				| (uint64_t)(mPtrState[0] & ~(uint16_t)maskState);
			return reinterpret_cast<Item*>(intPtr);
		}

		uint8_t GetState() const MOMO_NOEXCEPT
		{
			return (uint8_t)mPtrState[0] & maskState;
		}

	private:
		uint16_t mPtrState[3];
	};

	template<typename TItem, uint8_t tMaskState, size_t tBitCount>
	class BucketLimP4PtrState<TItem, tMaskState, tBitCount,
		typename std::enable_if<(48 < tBitCount && tBitCount <= 64)>::type>
	{
	public:
		typedef TItem Item;

		static const uint8_t maskState = tMaskState;
		static const size_t bitCount = 64;

	public:
		void Set(Item* ptr, uint8_t state) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(state <= maskState);
			uint64_t intPtr = reinterpret_cast<uint64_t>(ptr);
			MOMO_ASSERT(((uint8_t)intPtr & maskState) == 0);
			mPtrState[0] = (uint32_t)intPtr | (uint32_t)state;
			mPtrState[1] = (uint32_t)(intPtr >> 32);
		}

		Item* GetPointer() const MOMO_NOEXCEPT
		{
			uint64_t intPtr = ((uint64_t)mPtrState[1] << 32)
				| (uint64_t)(mPtrState[0] & ~(uint32_t)maskState);
			return reinterpret_cast<Item*>(intPtr);
		}

		uint8_t GetState() const MOMO_NOEXCEPT
		{
			return (uint8_t)mPtrState[0] & maskState;
		}

	private:
		uint32_t mPtrState[2];
	};

	template<typename TItemTraits, size_t tMaxCount, typename TMemPoolParams,
		bool tUseHashCodePartGetter>
	class BucketLimP4
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;

		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount <= 4);

		static const bool useHashCodePartGetter = tUseHashCodePartGetter;

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef Item* Iterator;
		typedef BucketBounds<Iterator> Bounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		static const size_t itemAlignment = (!useHashCodePartGetter || ItemTraits::alignment > 4)
			? ItemTraits::alignment : 4;

		static const size_t minMemPoolIndex =
			(maxCount > 1 && sizeof(Item) <= itemAlignment) ? 2 : 1;

		template<size_t memPoolIndex>
		using MemPoolParamsStatic = momo::MemPoolParamsStatic<memPoolIndex * sizeof(Item),
			itemAlignment, MemPoolParams::blockCount,
			(/*minMemPoolIndex <= memPoolIndex &&*/ memPoolIndex <= maxCount)	// vs2013
				? MemPoolParams::cachedFreeBlockCount : 0>;

		template<size_t memPoolIndex>
		using MemPool = momo::MemPool<MemPoolParamsStatic<memPoolIndex>, MemManagerPtr,
			NestedMemPoolSettings>;

		template<size_t memPoolIndex>
		using Memory = BucketMemory<MemPool<memPoolIndex>, Item*>;

		typedef BucketLimP4PtrState<Item, useHashCodePartGetter ? 3 : 0,
			MemManager::ptrUsefulBitCount> PtrState;

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

			~Params() MOMO_NOEXCEPT
			{
			}

			Params& operator=(const Params&) = delete;

			MemManager& GetMemManager() MOMO_NOEXCEPT
			{
				return std::get<0>(mMemPools).GetMemManager().GetBaseMemManager();
			}

			template<size_t memPoolIndex>
			MemPool<memPoolIndex>& GetMemPool() MOMO_NOEXCEPT
			{
				return std::get<memPoolIndex - 1>(mMemPools);
			}

		private:
			MemPools mMemPools;
		};

	public:
		BucketLimP4() MOMO_NOEXCEPT
		{
			pvSetEmpty(minMemPoolIndex);
		}

		BucketLimP4(const BucketLimP4&) = delete;

		~BucketLimP4() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mPtrState.GetPointer() == nullptr);
		}

		BucketLimP4& operator=(const BucketLimP4&) = delete;

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return Bounds(mPtrState.GetPointer(), pvGetCount());
		}

		template<typename Predicate>
		Iterator Find(Params& /*params*/, const Predicate& pred, size_t hashCode,
			size_t /*logBucketCount*/)
		{
			uint8_t hashByte = pvGetHashByte(hashCode);
			Item* items = mPtrState.GetPointer();
			for (size_t i = 0; i < maxCount; ++i)
			{
				if (mHashes[i] == hashByte && pred(items[i]))
					return items + i;
			}
			return nullptr;
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return mHashes[maxCount - 1] < maskEmpty;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return pvGetMemPoolIndex() == maxCount;
		}

		size_t GetMaxProbe(size_t logBucketCount) const MOMO_NOEXCEPT
		{
			return ((size_t)1 << logBucketCount) - 1;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
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
		Iterator AddCrt(Params& params, const ItemCreator& itemCreator, size_t hashCode,
			size_t logBucketCount, size_t probe)
		{
			Item* items = mPtrState.GetPointer();
			size_t memPoolIndex = pvGetMemPoolIndex();
			if (items == nullptr)
			{
				MOMO_ASSERT(memPoolIndex == minMemPoolIndex || memPoolIndex == maxCount);
				pvSetHashProbe(0, hashCode, logBucketCount, probe);
				if (memPoolIndex == minMemPoolIndex)
					return pvAdd0<minMemPoolIndex>(params, itemCreator, hashCode);
				else
					return pvAdd0<maxCount>(params, itemCreator, hashCode);
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
						return pvAdd<1>(params, itemCreator, hashCode, items);
					case 2:
						pvSetHashProbe(2, hashCode, logBucketCount, probe);
						return pvAdd<2>(params, itemCreator, hashCode, items);
					default:
						MOMO_ASSERT(memPoolIndex == 3);
						pvSetHashProbe(3, hashCode, logBucketCount, probe);
						return pvAdd<3>(params, itemCreator, hashCode, items);
					}
				}
				else
				{
					pvSetHashProbe(count, hashCode, logBucketCount, probe);
					itemCreator(items + count);
					mHashes[count] = pvGetHashByte(hashCode);
					pvSetPtrState(items, memPoolIndex);
					return items + count;
				}
			}
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& params, Iterator iter, const ItemReplacer& itemReplacer)
		{
			Item* items = mPtrState.GetPointer();
			MOMO_ASSERT(items != nullptr);
			size_t count = pvGetCount();
			size_t memPoolIndex = pvGetMemPoolIndex();
			if (count == 1)
			{
				MOMO_ASSERT(iter == items);
				itemReplacer(*items, *items);
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
				itemReplacer(items[count - 1], *iter);
				mHashes[index] = mHashes[count - 1];
				mHashes[count - 1] = emptyHashProbe;
				if (useHashCodePartGetter && hashCount - 1 - index >= count)
					mHashes[hashCount - 1 - index] = emptyHashProbe;	//?
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
			size_t hashProbe = (size_t)mHashes[hashCount - 1 - index];
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
				| ((size_t)mHashes[index] << hashCodeShift);
		}

	private:
		void pvSetEmpty(size_t memPoolIndex) MOMO_NOEXCEPT
		{
			std::fill_n(mHashes, hashCount, (uint8_t)emptyHashProbe);
			pvSetPtrState(nullptr, memPoolIndex);
		}

		void pvSetPtrState(Item* items, size_t memPoolIndex) MOMO_NOEXCEPT
		{
			uint8_t memPoolIndex1 = (uint8_t)memPoolIndex - 1;
			mPtrState.Set(items, useHashCodePartGetter ? memPoolIndex1 : 0);
			if (!useHashCodePartGetter && (maxCount < 4 || !IsFull()))
				mHashes[3] = maskEmpty + memPoolIndex1;
		}

		size_t pvGetMemPoolIndex() const MOMO_NOEXCEPT
		{
			if (useHashCodePartGetter)
				return (size_t)mPtrState.GetState() + 1;
			else if (maxCount == 4 && IsFull())
				return 4;
			else
				return (size_t)(mHashes[3] & 3) + 1;
		}

		size_t pvGetCount() const MOMO_NOEXCEPT
		{
			return (mHashes[1] >= maskEmpty) ? ((mHashes[0] < maskEmpty) ? 1 : 0)
				: 2 + ((mHashes[2] < maskEmpty) ? 1 : 0) + ((mHashes[3] < maskEmpty) ? 1 : 0);
		}

		static uint8_t pvGetHashByte(size_t hashCode) MOMO_NOEXCEPT
		{
			return (uint8_t)(hashCode >> hashCodeShift);
		}

		static size_t pvGetProbeShift(size_t logBucketCount) MOMO_NOEXCEPT
		{
			return (logBucketCount + logBucketCountAddend) % logBucketCountStep;
		}

		void pvSetHashProbe(size_t index, size_t hashCode, size_t logBucketCount,
			size_t probe) MOMO_NOEXCEPT
		{
			if (!useHashCodePartGetter || hashCount - 1 - index <= index)
				return;
			size_t probeShift = pvGetProbeShift(logBucketCount);
			mHashes[hashCount - 1 - index] = (probe < (size_t)1 << probeShift)
				? maskEmpty | (uint8_t)((hashCode >> logBucketCount) << probeShift) | (uint8_t)probe
				: emptyHashProbe;
		}

		template<size_t memPoolIndex, typename ItemCreator>
		Item* pvAdd0(Params& params, const ItemCreator& itemCreator, size_t hashCode)
		{
			Memory<memPoolIndex> memory(params.template GetMemPool<memPoolIndex>());
			Item* items = memory.GetPointer();
			itemCreator(items);
			mHashes[0] = pvGetHashByte(hashCode);
			pvSetPtrState(memory.Extract(), memPoolIndex);
			return items;
		}

		template<size_t memPoolIndex, typename ItemCreator>
		Item* pvAdd(Params& params, const ItemCreator& itemCreator, size_t hashCode, Item* items)
		{
			static const size_t newMemPoolIndex = memPoolIndex + 1;
			size_t count = memPoolIndex;
			Memory<newMemPoolIndex> memory(params.template GetMemPool<newMemPoolIndex>());
			Item* newItems = memory.GetPointer();
			ItemTraits::RelocateCreate(params.GetMemManager(), items, newItems, count,
				itemCreator, newItems + count);
			params.template GetMemPool<memPoolIndex>().Deallocate(items);
			mHashes[count] = pvGetHashByte(hashCode);
			pvSetPtrState(memory.Extract(), newMemPoolIndex);
			return newItems + count;
		}

		void pvDeallocate(Params& params, size_t memPoolIndex, Item* items) MOMO_NOEXCEPT
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
		uint8_t mHashes[hashCount];
	};
}

template<size_t tMaxCount = 4,
	typename TMemPoolParams = MemPoolParams<>,
	bool tUseHashCodePartGetter = true>
struct HashBucketLimP4 : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;
	static const bool useHashCodePartGetter = tUseHashCodePartGetter;

	typedef TMemPoolParams MemPoolParams;

	template<typename ItemTraits>
	using Bucket = internal::BucketLimP4<ItemTraits, maxCount, MemPoolParams,
		useHashCodePartGetter && sizeof(typename ItemTraits::Item) >= 4>;
};

} // namespace momo

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
	template<typename Item, size_t size = sizeof(Item*)>
	class BucketLimP4PtrState;

	template<typename Item>
	class BucketLimP4PtrState<Item, 4>
	{
	public:
		void Set(Item* ptr, uint8_t state) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(state < 4);
			mPtrState = reinterpret_cast<uint32_t>(ptr);
			MOMO_ASSERT(mPtrState % 4 == 0);
			mPtrState |= (uint32_t)state;
		}

		Item* GetPointer() const MOMO_NOEXCEPT
		{
			return reinterpret_cast<Item*>(mPtrState & ~(uint32_t)3);
		}

		uint8_t GetState() const MOMO_NOEXCEPT
		{
			return (uint8_t)(mPtrState & (uint32_t)3);
		}

	private:
		uint32_t mPtrState;
	};

	template<typename Item>
	class BucketLimP4PtrState<Item, 8>
	{
	public:
		void Set(Item* ptr, uint8_t state) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(state < 4);
			uint64_t intPtr = reinterpret_cast<uint64_t>(ptr);
			MOMO_ASSERT(intPtr % 4 == 0);
			mPtrState1 = (uint32_t)(intPtr) | (uint32_t)state;
			mPtrState2 = (uint32_t)(intPtr >> 32);
		}

		Item* GetPointer() const MOMO_NOEXCEPT
		{
			uint64_t intPtr = ((uint64_t)mPtrState2 << 32) | (uint64_t)(mPtrState1 & ~(uint32_t)3);
			return reinterpret_cast<Item*>(intPtr);
		}

		uint8_t GetState() const MOMO_NOEXCEPT
		{
			return (uint8_t)(mPtrState1 & (uint32_t)3);
		}

	private:
		uint32_t mPtrState1;
		uint32_t mPtrState2;
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

		static const size_t itemAlignment = (ItemTraits::alignment > 4) ? ItemTraits::alignment : 4;

		template<size_t memPoolIndex>
		using MemPoolParamsStatic = momo::MemPoolParamsStatic<memPoolIndex * sizeof(Item),
			itemAlignment, MemPoolParams::blockCount,
			(memPoolIndex <= maxCount) ? MemPoolParams::cachedFreeBlockCount : 0>;

		template<size_t memPoolIndex>
		using MemPool = momo::MemPool<MemPoolParamsStatic<memPoolIndex>, MemManagerPtr,
			NestedMemPoolSettings>;

		template<size_t memPoolIndex>
		using Memory = BucketMemory<MemPool<memPoolIndex>, Item*>;

		typedef BucketLimP4PtrState<Item> PtrState;

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
			pvSetState0(1);
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

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			Item* items = mPtrState.GetPointer();
			if (items != nullptr)
			{
				ItemTraits::Destroy(params.GetMemManager(), items, pvGetCount());
				pvDeallocate(params, pvGetMemPoolIndex(), items);
			}
			pvSetState0(1);
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& params, const ItemCreator& itemCreator, size_t hashCode,
			size_t logBucketCount, size_t probe)
		{
			Item* items = mPtrState.GetPointer();
			size_t memPoolIndex = pvGetMemPoolIndex();
			if (items == nullptr)
			{
				MOMO_ASSERT(memPoolIndex == 1 || memPoolIndex == maxCount);
				pvSetHashProbe(0, hashCode, logBucketCount, probe);
				if (memPoolIndex == 1)
					return pvAdd0<1>(params, itemCreator, hashCode);
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
						return pvAdd<2>(params, itemCreator, hashCode, items);
					default:
						MOMO_ASSERT(memPoolIndex == 3);
						return pvAdd<3>(params, itemCreator, hashCode, items);
					}
				}
				else
				{
					if (count == 1)
						pvSetHashProbe(1, hashCode, logBucketCount, probe);
					itemCreator(items + count);
					mHashes[count] = pvGetHashByte(hashCode);
					pvSetState(items, memPoolIndex);
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
					memPoolIndex = 1;
				pvSetState0(memPoolIndex);
				return nullptr;
			}
			else
			{
				size_t index = iter - items;
				MOMO_ASSERT(index < count);
				itemReplacer(items[count - 1], *iter);
				mHashes[index] = mHashes[count - 1];
				mHashes[count - 1] = emptyHashProbe;
				if (useHashCodePartGetter && index == 0)
					mHashes[3] = (count == 2) ? mHashes[2] : emptyHashProbe;
				pvSetState(items, memPoolIndex);
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
			size_t hashProbe = (size_t)mHashes[3 - index];
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
		void pvSetState0(size_t memPoolIndex) MOMO_NOEXCEPT
		{
			std::fill_n(mHashes, 4, (uint8_t)emptyHashProbe);
			pvSetState(nullptr, memPoolIndex);
		}

		void pvSetState(Item* items, size_t memPoolIndex) MOMO_NOEXCEPT
		{
			mPtrState.Set(items, (uint8_t)memPoolIndex - 1);
		}

		size_t pvGetMemPoolIndex() const MOMO_NOEXCEPT
		{
			return (size_t)mPtrState.GetState() + 1;
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
			if (!useHashCodePartGetter)
				return;
			size_t probeShift = pvGetProbeShift(logBucketCount);
			mHashes[3 - index] = (probe < (size_t)1 << probeShift)
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
			pvSetState(memory.Extract(), memPoolIndex);
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
			pvSetState(memory.Extract(), newMemPoolIndex);
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
		uint8_t mHashes[4];
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
		useHashCodePartGetter>;
};

} // namespace momo

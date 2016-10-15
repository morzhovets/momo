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
	public:
		typedef TItemTraits ItemTraits;
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		static const size_t logMaxCount = tLogMaxCount;
		MOMO_STATIC_ASSERT(0 < logMaxCount && logMaxCount <= 4);	//?

		static const size_t memPoolBlockCount = tMemPoolBlockCount;

		typedef BucketBounds<Item> Bounds;
		typedef typename Bounds::ConstBounds ConstBounds;

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
			typedef ArrayIntCap<maxCount, MemPool, MemManagerDummy> MemPools;

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

			const MemPool& GetMemPool(size_t memPoolIndex) const MOMO_NOEXCEPT
			{
				MOMO_ASSERT(memPoolIndex > 0);
				return mMemPools[memPoolIndex - 1];
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
			MOMO_ASSERT(_IsEmpty());
		}

		BucketLim4& operator=(const BucketLim4&) = delete;

		ConstBounds GetBounds(const Params& params) const MOMO_NOEXCEPT
		{
			return _GetBounds<ConstBounds>(params);
		}

		Bounds GetBounds(Params& params) MOMO_NOEXCEPT
		{
			return _GetBounds<Bounds>(params);
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			if (_IsEmpty())
				return false;
			return _GetData().count == maxCount;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			if (mPtrState == stateNull)
				return false;
			if (mPtrState == stateNullWasFull)
				return true;
			return _GetMemPoolIndex() == _GetMemPoolIndex(maxCount);
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (!_IsEmpty())
			{
				Data data = _GetData();
				uint32_t ptr = data.pointer;
				size_t memPoolIndex = _GetMemPoolIndex();
				MemPool& memPool = params.GetMemPool(memPoolIndex);
				Item* items = memPool.template GetRealPointer<Item>(ptr);
				ItemTraits::Destroy(params.GetMemManager(), items, data.count);
				memPool.Deallocate(ptr);
			}
			mPtrState = stateNull;
		}

		template<typename ItemCreator>
		Item* AddBackCrt(Params& params, const ItemCreator& itemCreator)
		{
			if (_IsEmpty())
			{
				size_t newCount = 1;
				size_t newMemPoolIndex =
					_GetMemPoolIndex((mPtrState == stateNull) ? newCount : maxCount);
				MemPool& newMemPool = params.GetMemPool(newMemPoolIndex);
				Memory memory(newMemPool);
				Item* newItems = newMemPool.template GetRealPointer<Item>(memory.GetPointer());
				itemCreator(newItems);
				_Set(memory.Extract(), newMemPoolIndex, newCount);
				return newItems;
			}
			else
			{
				Data data = _GetData();
				size_t count = data.count;
				uint32_t ptr = data.pointer;
				size_t memPoolIndex = _GetMemPoolIndex();
				MOMO_ASSERT(count <= memPoolIndex);
				MOMO_ASSERT(count < maxCount);
				MemPool& memPool = params.GetMemPool(memPoolIndex);
				Item* items = memPool.template GetRealPointer<Item>(ptr);
				if (count == memPoolIndex)
				{
					size_t newCount = count + 1;
					size_t newMemPoolIndex = _GetMemPoolIndex(newCount);
					MemPool& newMemPool = params.GetMemPool(newMemPoolIndex);
					Memory memory(newMemPool);
					Item* newItems = newMemPool.template GetRealPointer<Item>(memory.GetPointer());
					ItemTraits::RelocateCreate(params.GetMemManager(), items, newItems, count,
						itemCreator, newItems + count);
					memPool.Deallocate(ptr);
					_Set(memory.Extract(), newMemPoolIndex, newCount);
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
			MOMO_ASSERT(!_IsEmpty());
			Data data = _GetData();
			if (data.count == 1)
			{
				size_t memPoolIndex = _GetMemPoolIndex();
				params.GetMemPool(memPoolIndex).Deallocate(data.pointer);
				mPtrState = (memPoolIndex < _GetMemPoolIndex(maxCount))
					? stateNull : stateNullWasFull;
			}
			else
			{
				--mPtrState;
			}
		}

	private:
		bool _IsEmpty() const MOMO_NOEXCEPT
		{
			return mPtrState == stateNull || mPtrState == stateNullWasFull;
		}

		void _Set(uint32_t ptr, size_t memPoolIndex, size_t count)
		{
			mPtrState = (uint32_t)(((memPoolIndex - 1) << (32 - logMaxCount))
				+ (size_t)ptr * memPoolIndex + count - 1);
		}

		static size_t _GetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(0 < count && count <= maxCount);
			return count;
		}

		size_t _GetMemPoolIndex() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!_IsEmpty());
			return (size_t)(mPtrState >> (32 - logMaxCount)) + 1;
		}

		Data _GetData() const MOMO_NOEXCEPT
		{
			UIntMath<uint32_t>::DivResult divRes = UIntMath<uint32_t>::DivBySmall(
				mPtrState & stateNull, (uint32_t)_GetMemPoolIndex());
			Data data;
			data.pointer = divRes.quotient;
			data.count = (size_t)divRes.remainder + 1;
			return data;
		}

		template<typename Bounds, typename Params>
		Bounds _GetBounds(Params& params) const MOMO_NOEXCEPT
		{
			if (_IsEmpty())
				return Bounds();
			Data data = _GetData();
			auto& memPool = params.GetMemPool(_GetMemPoolIndex());
			auto* items = memPool.template GetRealPointer<typename Bounds::Item>(data.pointer);
			return Bounds(items, data.count);
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

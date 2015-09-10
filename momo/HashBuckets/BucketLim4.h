/**********************************************************\

  momo/HashBuckets/BucketLim4.h

  namespace momo:
    struct HashBucketLim4

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../MemPool.h"
#include "../Array.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, typename TMemManager,
		size_t tLogMaxCount, size_t tMemPoolBlockCount>
	class BucketLim4
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemManager MemManager;
		typedef typename ItemTraits::Item Item;

		static const size_t logMaxCount = tLogMaxCount;
		MOMO_STATIC_ASSERT(0 < logMaxCount && logMaxCount < 8);	//?

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

	public:
		class Params
		{
		private:
			typedef momo::Array<MemPool, MemManagerDummy, ArrayItemTraits<MemPool>,
				ArraySettings<maxCount>> MemPools;

		public:
			Params(MemManager& memManager)
			{
				for (size_t i = 1; i <= maxCount; ++i)
				{
					auto memPoolCreator = [i, &memManager] (void* pmemPool)
					{
						size_t maxTotalBlockCount = (i == 1)
							? ((size_t)1 << (32 - logMaxCount)) - 2
							: ((size_t)1 << (32 - logMaxCount)) / i;
						new(pmemPool) MemPool(i * sizeof(Item), MemManagerPtr(memManager),
							maxTotalBlockCount);
					};
					mMemPools.AddBackNogrowEmpl(memPoolCreator);
				}
			}

			const MemPool& operator[](size_t index) const MOMO_NOEXCEPT
			{
				return mMemPools[index];
			}

			MemPool& operator[](size_t index) MOMO_NOEXCEPT
			{
				return mMemPools[index];
			}

		private:
			MOMO_DISABLE_COPY_CONSTRUCTOR(Params);
			MOMO_DISABLE_COPY_OPERATOR(Params);

		private:
			MemPools mMemPools;
		};

	public:
		BucketLim4() MOMO_NOEXCEPT
			: mState(stateNull)
		{
		}

		~BucketLim4() MOMO_NOEXCEPT
		{
			assert(mState == stateNull);
		}

		ConstBounds GetBounds(const Params& params) const MOMO_NOEXCEPT
		{
			return ConstBounds(_GetItems<const Item>(params), _GetCount());
		}

		Bounds GetBounds(Params& params) MOMO_NOEXCEPT
		{
			return Bounds(_GetItems<Item>(params), _GetCount());
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			if (_IsNull())
				return false;
			return _GetCount() == maxCount;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			if (mState == stateNull)
				return false;
			if (mState == stateNullWasFull)
				return true;
			return _GetMemPoolIndex() == maxCount - 1;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (!_IsNull())
			{
				Item* items = _GetItems<Item>(params);
				ItemTraits::Destroy(items, _GetCount());
				params[_GetMemPoolIndex()].Deallocate(_GetPointer());
			}
			mState = stateNull;
		}

		template<typename ItemCreator>
		void AddBackEmpl(Params& params, const ItemCreator& itemCreator)
		{
			if (_IsNull())
			{
				size_t newCount = 1;
				size_t newMemPoolIndex = WasFull() ? maxCount - 1 : _GetMemPoolIndex(newCount);
				MemPool& newMemPool = params[newMemPoolIndex];
				Memory memory(newMemPool);
				Item* newItems = (Item*)newMemPool.GetRealPointer(memory.GetPointer());
				itemCreator(newItems);
				_Set(memory.Extract(), newMemPoolIndex, newCount);
			}
			else
			{
				size_t memPoolIndex = _GetMemPoolIndex();
				size_t count = _GetCount();
				assert(count <= memPoolIndex + 1);
				assert(count < maxCount);
				if (count == memPoolIndex + 1)
				{
					size_t newCount = count + 1;
					size_t newMemPoolIndex = _GetMemPoolIndex(newCount);
					MemPool& newMemPool = params[newMemPoolIndex];
					Memory memory(newMemPool);
					Item* newItems = (Item*)newMemPool.GetRealPointer(memory.GetPointer());
					MemPool& memPool = params[memPoolIndex];
					uint32_t ptr = _GetPointer();
					Item* items = (Item*)memPool.GetRealPointer(ptr);
					ItemTraits::RelocateAddBack(items, newItems, count, itemCreator);
					params[memPoolIndex].Deallocate(ptr);
					_Set(memory.Extract(), newMemPoolIndex, newCount);
				}
				else
				{
					itemCreator(_GetItems<Item>(params) + count);
					++mState;
				}
			}
		}

		void RemoveBack(Params& params) MOMO_NOEXCEPT
		{
			size_t count = _GetCount();
			assert(count > 0);
			Item* items = _GetItems<Item>(params);
			ItemTraits::Destroy(items + count - 1, 1);
			if (count == 1)
			{
				size_t memPoolIndex = _GetMemPoolIndex();
				params[memPoolIndex].Deallocate(_GetPointer());
				mState = (memPoolIndex < maxCount - 1) ? stateNull : stateNullWasFull;
			}
			else
			{
				--mState;
			}
		}

	private:
		bool _IsNull() const MOMO_NOEXCEPT
		{
			return mState == stateNull || mState == stateNullWasFull;
		}

		void _Set(uint32_t ptr, size_t memPoolIndex, size_t count)
		{
			mState = (uint32_t)(memPoolIndex << (32 - logMaxCount))
				| (uint32_t)(ptr * (memPoolIndex + 1) + count - 1);
		}

		static size_t _GetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			assert(0 < count && count <= maxCount);
			return count - 1;
		}

		size_t _GetMemPoolIndex() const MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return (size_t)(mState >> (32 - logMaxCount));
		}

		size_t _GetCount() const MOMO_NOEXCEPT
		{
			if (_IsNull())
				return 0;
			size_t memPoolIndex = _GetMemPoolIndex();
			size_t count = (size_t)(mState & stateNull) % (memPoolIndex + 1) + 1;
			return count;
		}

		int32_t _GetPointer() const MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			size_t memPoolIndex = _GetMemPoolIndex();
			uint32_t ptr = (mState & stateNull) / (uint32_t)(memPoolIndex + 1);
			return ptr;
		}

		template<typename Item, typename Params>
		Item* _GetItems(Params& params) const MOMO_NOEXCEPT
		{
			if (_IsNull())
				return nullptr;
			auto& memPool = params[_GetMemPoolIndex()];
			auto* realPtr = memPool.GetRealPointer(_GetPointer());
			return (Item*)realPtr;
		}

	private:
		MOMO_DISABLE_COPY_CONSTRUCTOR(BucketLim4);
		MOMO_DISABLE_COPY_OPERATOR(BucketLim4);

	private:
		uint32_t mState;
	};
}

template<size_t tLogMaxCount = 2,
	size_t tMemPoolBlockCount = MemPoolConst::defaultBlockCount>
struct HashBucketLim4 : public internal::HashBucketBase<1 << tLogMaxCount>
{
	static const size_t logMaxCount = tLogMaxCount;
	static const size_t memPoolBlockCount = tMemPoolBlockCount;

	template<typename ItemTraits, typename MemManager>
	struct Bucketer
	{
		typedef internal::BucketLim4<ItemTraits, MemManager,
			logMaxCount, memPoolBlockCount> Bucket;
	};
};

} // namespace momo

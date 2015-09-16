/**********************************************************\

  momo/HashBuckets/BucketFewP.h

  namespace momo:
    struct HashBucketFewP

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../MemPool.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, typename TMemManager,
		size_t tMaxCount, size_t tMemPoolBlockCount>
	class BucketFewP
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemManager MemManager;
		typedef typename ItemTraits::Item Item;

		static const size_t maxCount = tMaxCount;
		static const size_t memPoolBlockCount = tMemPoolBlockCount;

		typedef BucketBounds<Item> Bounds;
		typedef typename Bounds::ConstBounds ConstBounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		class MemPoolParams
		{
		public:
			static const size_t blockCount = memPoolBlockCount;
			MOMO_STATIC_ASSERT(0 < blockCount && blockCount < 128);

		public:
			MemPoolParams(size_t blockAlignment, size_t blockSize) MOMO_NOEXCEPT
			{
				this->blockAlignment = blockAlignment;
				this->blockSize = (blockCount == 1)
					? ((blockSize > 0) ? blockSize : 1)
					: ((blockSize <= blockAlignment)
						? 2 * blockAlignment
						: internal::UIntMath<size_t>::Ceil(blockSize, blockAlignment));
			}

		protected:
			size_t blockAlignment;
			size_t blockSize;
		};

		typedef momo::MemPool<MemPoolParams, MemManagerPtr> MemPool;

		typedef BucketMemory<MemPool, Item*> Memory;

		static const size_t itemAlignment = (ItemTraits::alignment > 4) ? ItemTraits::alignment
			: (maxCount > 2) ? 4 : (maxCount > 1) ? 2 : 1;
		MOMO_STATIC_ASSERT(maxCount <= itemAlignment);

		static const bool skipOddMemPools =
			(maxCount % 2 == 0 && memPoolBlockCount > 1 && sizeof(Item) <= itemAlignment);
		static const uintptr_t modMemPoolIndex = (uintptr_t)itemAlignment / (skipOddMemPools ? 2 : 1);

		static const uintptr_t ptrNull = 0;
		static const uintptr_t ptrNullWasFull = 1;

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
					if (skipOddMemPools && i % 2 == 1)
						continue;
					mMemPools.AddBackNogrow(MemPool(MemPoolParams(i * (size_t)modMemPoolIndex,
						i * sizeof(Item)), MemManagerPtr(memManager)));
				}
			}

			MemPool& GetMemPool(size_t memPoolIndex) MOMO_NOEXCEPT
			{
				assert(memPoolIndex > 0);
				return mMemPools[(memPoolIndex - 1) / (skipOddMemPools ? 2 : 1)];
			}

		private:
			MOMO_DISABLE_COPY_CONSTRUCTOR(Params);
			MOMO_DISABLE_COPY_OPERATOR(Params);

		private:
			MemPools mMemPools;
		};

	public:
		BucketFewP() MOMO_NOEXCEPT
			: mPtr(ptrNull)
		{
		}

		~BucketFewP() MOMO_NOEXCEPT
		{
			assert(mPtr == ptrNull);
		}

		ConstBounds GetBounds(const Params& /*params*/) const MOMO_NOEXCEPT
		{
			return _GetBounds();
		}

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return _GetBounds();
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return _GetBounds().GetCount() == maxCount;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			if (mPtr == ptrNull)
				return false;
			if (mPtr == ptrNullWasFull)
				return true;
			return _GetMemPoolIndex() == maxCount;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (!_IsNull())
			{
				Bounds bounds = _GetBounds();
				Item* items = bounds.GetBegin();
				ItemTraits::Destroy(items, bounds.GetCount());
				params.GetMemPool(_GetMemPoolIndex()).Deallocate(items);
			}
			mPtr = ptrNull;
		}

		template<typename ItemCreator>
		void AddBackEmpl(Params& params, const ItemCreator& itemCreator)
		{
			if (_IsNull())
			{
				size_t newCount = 1;
				size_t newMemPoolIndex = _GetMemPoolIndex((mPtr == ptrNull) ? newCount : maxCount);
				Memory memory(params.GetMemPool(newMemPoolIndex));
				itemCreator(memory.GetPointer());
				_Set(memory.Extract(), newMemPoolIndex, newCount);
			}
			else
			{
				size_t memPoolIndex = _GetMemPoolIndex();
				Bounds bounds = _GetBounds();
				size_t count = bounds.GetCount();
				Item* items = bounds.GetBegin();
				assert(count <= memPoolIndex);
				assert(count < maxCount);
				if (count == memPoolIndex)
				{
					size_t newCount = count + 1;
					size_t newMemPoolIndex = _GetMemPoolIndex(newCount);
					Memory memory(params.GetMemPool(newMemPoolIndex));
					ItemTraits::RelocateAddBack(items, memory.GetPointer(),
						count, itemCreator);
					params.GetMemPool(memPoolIndex).Deallocate(items);
					_Set(memory.Extract(), newMemPoolIndex, newCount);
				}
				else
				{
					itemCreator(items + count);
					mPtr += modMemPoolIndex;
				}
			}
		}

		void RemoveBack(Params& params) MOMO_NOEXCEPT
		{
			Bounds bounds = _GetBounds();
			size_t count = bounds.GetCount();
			assert(count > 0);
			Item* items = bounds.GetBegin();
			ItemTraits::Destroy(items + count - 1, 1);
			if (count == 1)
			{
				size_t memPoolIndex = _GetMemPoolIndex();
				params.GetMemPool(memPoolIndex).Deallocate(items);
				mPtr = (memPoolIndex < maxCount) ? ptrNull : ptrNullWasFull;
			}
			else
			{
				mPtr -= modMemPoolIndex;
			}
		}

	private:
		bool _IsNull() const MOMO_NOEXCEPT
		{
			return mPtr == ptrNull || mPtr == ptrNullWasFull;
		}

		void _Set(Item* items, size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			mPtr = (uintptr_t)items;
			assert(mPtr % (modMemPoolIndex * (uintptr_t)memPoolIndex) == 0);
			mPtr += (uintptr_t)(count - 1) * modMemPoolIndex
				+ (uintptr_t)memPoolIndex / (skipOddMemPools ? 2 : 1) - 1;
		}

		static size_t _GetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			assert(0 < count && count <= maxCount);
			return count + (skipOddMemPools ? count % 2 : 0);
		}

		size_t _GetMemPoolIndex() const MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return (size_t)(mPtr % modMemPoolIndex + 1) * (skipOddMemPools ? 2 : 1);
		}

		Bounds _GetBounds() const MOMO_NOEXCEPT
		{
			if (_IsNull())
				return Bounds();
			size_t memPoolIndex = _GetMemPoolIndex();
			uintptr_t ptrCount = mPtr / modMemPoolIndex;
			auto divResult = internal::UIntMath<uintptr_t>::DivBySmall(ptrCount, (uintptr_t)memPoolIndex);
			uintptr_t ptr = divResult.quotient;
			size_t count = (size_t)divResult.remainder + 1;
			Item* items = (Item*)(ptr * modMemPoolIndex * (uintptr_t)memPoolIndex);
			return Bounds(items, count);
		}

	private:
		MOMO_DISABLE_COPY_CONSTRUCTOR(BucketFewP);
		MOMO_DISABLE_COPY_OPERATOR(BucketFewP);

	private:
		uintptr_t mPtr;
	};
}

template<size_t tMaxCount = 4,
	size_t tMemPoolBlockCount = MemPoolConst::defaultBlockCount>
struct HashBucketFewP : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;
	static const size_t memPoolBlockCount = tMemPoolBlockCount;

	template<typename ItemTraits, typename MemManager>
	struct Bucketer
	{
		typedef internal::BucketFewP<ItemTraits, MemManager, maxCount, memPoolBlockCount> Bucket;
	};
};

} // namespace momo

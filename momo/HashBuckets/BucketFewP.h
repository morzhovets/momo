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

		static const uintptr_t itemAlignment =
			(ItemTraits::alignment > 4) ? ItemTraits::alignment : 4;
		MOMO_STATIC_ASSERT(maxCount <= (size_t)itemAlignment);

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
					mMemPools.AddBackNogrow(MemPool(MemPoolParams(i * (size_t)itemAlignment,
						i * sizeof(Item)), MemManagerPtr(memManager)));
				}
			}

			MemPool& GetMemPool(size_t memPoolIndex) MOMO_NOEXCEPT
			{
				assert(memPoolIndex > 0);
				return mMemPools[memPoolIndex - 1];
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
			return ConstBounds(_GetItems(), _GetCount());
		}

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return Bounds(_GetItems(), _GetCount());
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return _GetCount() == maxCount;
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
				Item* items = _GetItems();
				ItemTraits::Destroy(items, _GetCount());
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
				size_t newMemPoolIndex = (mPtr == ptrNull) ? _GetMemPoolIndex(newCount) : maxCount;
				Memory memory(params.GetMemPool(newMemPoolIndex));
				itemCreator(memory.GetPointer());
				_Set(memory.Extract(), newMemPoolIndex, newCount);
			}
			else
			{
				size_t memPoolIndex = _GetMemPoolIndex();
				size_t count = _GetCount();
				assert(count <= memPoolIndex);
				assert(count < maxCount);
				if (count == memPoolIndex)
				{
					size_t newCount = count + 1;
					size_t newMemPoolIndex = _GetMemPoolIndex(newCount);
					Memory memory(params.GetMemPool(newMemPoolIndex));
					Item* items = _GetItems();
					ItemTraits::RelocateAddBack(items, memory.GetPointer(),
						count, itemCreator);
					params.GetMemPool(memPoolIndex).Deallocate(items);
					_Set(memory.Extract(), newMemPoolIndex, newCount);
				}
				else
				{
					itemCreator(_GetItems() + count);
					mPtr += itemAlignment;
				}
			}
		}

		void RemoveBack(Params& params) MOMO_NOEXCEPT
		{
			size_t count = _GetCount();
			assert(count > 0);
			Item* items = _GetItems();
			ItemTraits::Destroy(items + count - 1, 1);
			if (count == 1)
			{
				size_t memPoolIndex = _GetMemPoolIndex();
				params.GetMemPool(memPoolIndex).Deallocate(items);
				mPtr = (memPoolIndex < maxCount) ? ptrNull : ptrNullWasFull;
			}
			else
			{
				mPtr -= itemAlignment;
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
			assert(mPtr % (itemAlignment * (uintptr_t)memPoolIndex) == 0);
			mPtr += (uintptr_t)(count - 1) * itemAlignment + (uintptr_t)memPoolIndex - 1;
		}

		static size_t _GetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			assert(0 < count && count <= maxCount);
			return count;
		}

		size_t _GetMemPoolIndex() const MOMO_NOEXCEPT
		{
			assert(!_IsNull());
			return (size_t)(mPtr % itemAlignment) + 1;
		}

		size_t _GetCount() const MOMO_NOEXCEPT
		{
			if (_IsNull())
				return 0;
			return (size_t)internal::UIntMath<uintptr_t>::ModSmall(mPtr / itemAlignment,
				(uintptr_t)_GetMemPoolIndex()) + 1;
		}

		Item* _GetItems() const MOMO_NOEXCEPT
		{
			if (_IsNull())
				return nullptr;
			size_t memPoolIndex = _GetMemPoolIndex();
			return (Item*)(internal::UIntMath<uintptr_t>::DivSmall(mPtr / itemAlignment,
				(uintptr_t)memPoolIndex) * (itemAlignment * (uintptr_t)memPoolIndex));
		}

	private:
		MOMO_DISABLE_COPY_CONSTRUCTOR(BucketFewP);
		MOMO_DISABLE_COPY_OPERATOR(BucketFewP);

	private:
		uintptr_t mPtr;
	};
}

template<size_t tMaxCount = sizeof(void*),
	size_t tMemPoolBlockCount = MemPoolConst::defaultBlockCount>
struct HashBucketFewP : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;
	static const size_t memPoolBlockCount = tMemPoolBlockCount;

	template<typename ItemTraits, typename MemManager>
	struct Bucketer
	{
		typedef internal::BucketFewP<ItemTraits, MemManager,
			maxCount, memPoolBlockCount> Bucket;
	};
};

} // namespace momo

/**********************************************************\

  momo/details/BucketLimP.h

  namespace momo:
    struct HashBucketLimP

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
		size_t tMaxCount, size_t tMemPoolBlockCount, bool tUseUIntPtr>
	class BucketLimP;

	template<typename TItemTraits, typename TMemManager,
		size_t tMaxCount, size_t tMemPoolBlockCount>
	class BucketLimP<TItemTraits, TMemManager, tMaxCount, tMemPoolBlockCount, false>
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemManager MemManager;
		typedef typename ItemTraits::Item Item;

		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount < 16);

		static const size_t memPoolBlockCount = tMemPoolBlockCount;
		static const bool useUIntPtr = false;

		typedef BucketBounds<Item> Bounds;
		typedef typename Bounds::ConstBounds ConstBounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef momo::MemPool<MemPoolParamsVarSize<ItemTraits::alignment, memPoolBlockCount>,
			MemManagerPtr> MemPool;

		typedef BucketMemory<MemPool, unsigned char*> Memory;

		static const uintptr_t ptrNull = UIntPtrConst::null;
		static const uintptr_t ptrNullWasFull = UIntPtrConst::invalid;

	public:
		class Params
		{
		private:
			typedef momo::Array<MemPool, MemManagerDummy, ArrayItemTraits<MemPool>,
				ArraySettings<maxCount>> MemPools;

		public:
			explicit Params(MemManager& memManager)
			{
				for (size_t i = 1; i <= maxCount; ++i)
				{
					size_t blockSize = i * sizeof(Item) + ItemTraits::alignment;
					mMemPools.AddBackNogrow(MemPool(typename MemPool::Params(blockSize),
						MemManagerPtr(memManager)));
				}
			}

			Params(const Params&) = delete;

			~Params() MOMO_NOEXCEPT
			{
			}

			Params& operator=(const Params&) = delete;

			MemPool& GetMemPool(size_t memPoolIndex) MOMO_NOEXCEPT
			{
				assert(memPoolIndex > 0);
				return mMemPools[memPoolIndex - 1];
			}

		private:
			MemPools mMemPools;
		};

	public:
		BucketLimP() MOMO_NOEXCEPT
			: mPtr(ptrNull)
		{
		}

		BucketLimP(const BucketLimP&) = delete;

		~BucketLimP() MOMO_NOEXCEPT
		{
			assert(_IsEmpty());
		}

		BucketLimP& operator=(const BucketLimP&) = delete;

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
			if (_IsEmpty())
				return false;
			return _GetCount() == maxCount;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			if (mPtr == ptrNull)
				return false;
			if (mPtr == ptrNullWasFull)
				return true;
			return _GetMemPoolIndex() == _GetMemPoolIndex(maxCount);
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (!_IsEmpty())
			{
				ItemTraits::Destroy(_GetItems(), _GetCount());
				params.GetMemPool(_GetMemPoolIndex()).Deallocate(_GetPtr());
			}
			mPtr = ptrNull;
		}

		template<typename ItemCreator>
		Item* AddBackCrt(Params& params, const ItemCreator& itemCreator)
		{
			if (_IsEmpty())
			{
				size_t newCount = 1;
				size_t newMemPoolIndex = _GetMemPoolIndex((mPtr == ptrNull) ? newCount : maxCount);
				Memory memory(params.GetMemPool(newMemPoolIndex));
				Item* newItems = _GetItems(memory.GetPointer());
				itemCreator(newItems);
				_Set(memory.Extract(), newMemPoolIndex, newCount);
				return newItems;
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
					Item* newItems = _GetItems(memory.GetPointer());
					ItemTraits::RelocateAddBack(_GetItems(), newItems, count, itemCreator);
					params.GetMemPool(memPoolIndex).Deallocate(_GetPtr());
					_Set(memory.Extract(), newMemPoolIndex, newCount);
					return newItems + count;
				}
				else
				{
					Item* items = _GetItems();
					itemCreator(items + count);
					++*_GetPtr();
					return items + count;
				}
			}
		}

		void RemoveBack(Params& params) MOMO_NOEXCEPT
		{
			size_t count = _GetCount();
			assert(count > 0);
			ItemTraits::Destroy(_GetItems() + count - 1, 1);
			if (count == 1)
			{
				size_t memPoolIndex = _GetMemPoolIndex();
				params.GetMemPool(memPoolIndex).Deallocate(_GetPtr());
				mPtr = (memPoolIndex < _GetMemPoolIndex(maxCount)) ? ptrNull : ptrNullWasFull;
			}
			else
			{
				--*_GetPtr();
			}
		}

	private:
		bool _IsEmpty() const MOMO_NOEXCEPT
		{
			return mPtr == ptrNull || mPtr == ptrNullWasFull;
		}

		unsigned char* _GetPtr() const MOMO_NOEXCEPT
		{
			assert(!_IsEmpty());
			return (unsigned char*)mPtr;
		}

		void _Set(unsigned char* ptr, size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			assert(ptr != nullptr);
			mPtr = (uintptr_t)ptr;
			*ptr = (unsigned char)((memPoolIndex << 4) | count);
		}

		static size_t _GetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			assert(0 < count && count <= maxCount);
			return count;
		}

		size_t _GetMemPoolIndex() const MOMO_NOEXCEPT
		{
			return (size_t)(*_GetPtr() >> 4);
		}

		size_t _GetCount() const MOMO_NOEXCEPT
		{
			return (size_t)(*_GetPtr() & 15);
		}

		Item* _GetItems() const MOMO_NOEXCEPT
		{
			return _GetItems(_GetPtr());
		}

		static Item* _GetItems(unsigned char* ptr) MOMO_NOEXCEPT
		{
			return (Item*)(ptr + ItemTraits::alignment);
		}

		Bounds _GetBounds() const MOMO_NOEXCEPT
		{
			if (_IsEmpty())
				return Bounds();
			else
				return Bounds(_GetItems(), _GetCount());
		}

	private:
		uintptr_t mPtr;
	};

	template<typename TItemTraits, typename TMemManager,
		size_t tMaxCount, size_t tMemPoolBlockCount>
	class BucketLimP<TItemTraits, TMemManager, tMaxCount, tMemPoolBlockCount, true>
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemManager MemManager;
		typedef typename ItemTraits::Item Item;

		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount < 16);

		static const size_t memPoolBlockCount = tMemPoolBlockCount;
		static const bool useUIntPtr = true;

		typedef BucketBounds<Item> Bounds;
		typedef typename Bounds::ConstBounds ConstBounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		class MemPoolParams	//?
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

		static const size_t minItemAlignment = (maxCount <= 1) ? 1 : (maxCount <= 2) ? 2
			: (maxCount <= 4) ? 4 : (maxCount <= 8) ? 8 : 16;
		static const size_t itemAlignment = (ItemTraits::alignment < minItemAlignment)
			? minItemAlignment : ItemTraits::alignment;

		static const bool skipOddMemPools =
			(maxCount > 1 && memPoolBlockCount > 1 && sizeof(Item) <= itemAlignment);
		static const uintptr_t modMemPoolIndex =
			(uintptr_t)minItemAlignment / (skipOddMemPools ? 2 : 1);

		static const uintptr_t stateNull = UIntPtrConst::null;
		static const uintptr_t stateNullWasFull = UIntPtrConst::invalid;

	public:
		class Params
		{
		private:
			typedef momo::Array<MemPool, MemManagerDummy, ArrayItemTraits<MemPool>,
				ArraySettings<maxCount>> MemPools;

		public:
			explicit Params(MemManager& memManager)
			{
				for (size_t i = 1; i <= maxCount + (skipOddMemPools ? 1 : 0); ++i)
				{
					if (skipOddMemPools && i % 2 == 1)
						continue;
					size_t blockAlignment = internal::UIntMath<size_t>::Ceil(
						i * (size_t)modMemPoolIndex, itemAlignment);
					mMemPools.AddBackNogrow(MemPool(MemPoolParams(blockAlignment, i * sizeof(Item)),
						MemManagerPtr(memManager)));
				}
			}

			Params(const Params&) = delete;

			~Params() MOMO_NOEXCEPT
			{
			}

			Params& operator=(const Params&) = delete;

			MemPool& GetMemPool(size_t memPoolIndex) MOMO_NOEXCEPT
			{
				assert(memPoolIndex > 0);
				return mMemPools[(memPoolIndex - 1) / (skipOddMemPools ? 2 : 1)];
			}

		private:
			MemPools mMemPools;
		};

	public:
		BucketLimP() MOMO_NOEXCEPT
			: mPtrState(stateNull)
		{
		}

		BucketLimP(const BucketLimP&) = delete;

		~BucketLimP() MOMO_NOEXCEPT
		{
			assert(_IsEmpty());
		}

		BucketLimP& operator=(const BucketLimP&) = delete;

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
				Bounds bounds = _GetBounds();
				Item* items = bounds.GetBegin();
				ItemTraits::Destroy(items, bounds.GetCount());
				params.GetMemPool(_GetMemPoolIndex()).Deallocate(items);
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
				Memory memory(params.GetMemPool(newMemPoolIndex));
				Item* newItems = memory.GetPointer();
				itemCreator(newItems);
				_Set(memory.Extract(), newMemPoolIndex, newCount);
				return newItems;
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
					Item* newItems = memory.GetPointer();
					ItemTraits::RelocateAddBack(items, newItems, count, itemCreator);
					params.GetMemPool(memPoolIndex).Deallocate(items);
					_Set(memory.Extract(), newMemPoolIndex, newCount);
					return newItems + count;
				}
				else
				{
					itemCreator(items + count);
					mPtrState += modMemPoolIndex;
					return items + count;
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
				mPtrState = (memPoolIndex < _GetMemPoolIndex(maxCount))
					? stateNull : stateNullWasFull;
			}
			else
			{
				mPtrState -= modMemPoolIndex;
			}
		}

	private:
		bool _IsEmpty() const MOMO_NOEXCEPT
		{
			return mPtrState == stateNull || mPtrState == stateNullWasFull;
		}

		void _Set(Item* items, size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			mPtrState = (uintptr_t)items + (uintptr_t)(count - 1) * modMemPoolIndex
				+ (uintptr_t)memPoolIndex / (skipOddMemPools ? 2 : 1) - 1;
		}

		static size_t _GetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			assert(0 < count && count <= maxCount);
			return count + (skipOddMemPools ? count % 2 : 0);
		}

		size_t _GetMemPoolIndex() const MOMO_NOEXCEPT
		{
			assert(!_IsEmpty());
			return (size_t)((mPtrState % modMemPoolIndex) + 1) * (skipOddMemPools ? 2 : 1);
		}

		Bounds _GetBounds() const MOMO_NOEXCEPT
		{
			if (_IsEmpty())
				return Bounds();
			uintptr_t memPoolIndex = (uintptr_t)_GetMemPoolIndex();
			uintptr_t ptrCount = mPtrState / modMemPoolIndex;
			typedef internal::UIntMath<uintptr_t> PMath;
			uintptr_t mod = PMath::Ceil(memPoolIndex, (uintptr_t)itemAlignment / modMemPoolIndex);
			uintptr_t count1 = PMath::DivBySmall(ptrCount, mod).remainder;
			assert(count1 < memPoolIndex);
			Item* items = (Item*)((ptrCount - count1) * modMemPoolIndex);
			return Bounds(items, (size_t)count1 + 1);
		}

	private:
		uintptr_t mPtrState;
	};
}

template<size_t tMaxCount = sizeof(void*),
	size_t tMemPoolBlockCount = MemPoolConst::defaultBlockCount,
	bool tUseUIntPtr = true>
struct HashBucketLimP : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;
	static const size_t memPoolBlockCount = tMemPoolBlockCount;
	static const bool useUIntPtr = tUseUIntPtr;

private:
	template<typename ItemTraits, typename MemManager>
	struct Bucketer
	{
	private:
		static const size_t size = sizeof(typename ItemTraits::Item);
		static const size_t alignment = ItemTraits::alignment;

		static const bool useUIntPtr = HashBucketLimP::useUIntPtr && ((maxCount <= 1)
			|| (maxCount <= 2 && size % 2 == 0 && (size > 2 || alignment == 2))
			|| (maxCount <= 4 && size % 4 == 0 && (size > 4 || alignment == 4))
			|| (maxCount <= 8 && size % 8 == 0 && (size > 8 || alignment == 8))
			|| (maxCount <= 16 && size % 16 == 0 && (size > 16 || alignment == 16)));

	public:
		typedef internal::BucketLimP<ItemTraits, MemManager,
			maxCount, memPoolBlockCount, useUIntPtr> Bucket;
	};

public:
	template<typename ItemTraits, typename MemManager>
	using Bucket = typename Bucketer<ItemTraits, MemManager>::Bucket;
};

} // namespace momo

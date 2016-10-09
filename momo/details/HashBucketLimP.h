/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketLimP.h

  namespace momo:
    struct HashBucketLimP

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../MemPool.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, size_t tMaxCount, typename TMemPoolParams, bool tUseUIntPtr>
	class BucketLimP;

	template<typename TItemTraits, size_t tMaxCount, typename TMemPoolParams>
	class BucketLimP<TItemTraits, tMaxCount, TMemPoolParams, false>
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount < 16);

		static const bool useUIntPtr = false;

		typedef BucketBounds<Item> Bounds;
		typedef typename Bounds::ConstBounds ConstBounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef momo::MemPool<MemPoolParams, MemManagerPtr> MemPool;

		typedef BucketMemory<MemPool, unsigned char*> Memory;

		static const uintptr_t ptrNull = UIntPtrConst::null;
		static const uintptr_t ptrNullWasFull = UIntPtrConst::invalid;

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
					size_t blockSize = i * sizeof(Item) + ItemTraits::alignment;
					mMemPools.AddBackNogrow(MemPool(MemPoolParams(blockSize, ItemTraits::alignment),
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
				MOMO_ASSERT(memPoolIndex > 0);
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
			MOMO_ASSERT(_IsEmpty());
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
				MOMO_ASSERT(count <= memPoolIndex);
				MOMO_ASSERT(count < maxCount);
				if (count == memPoolIndex)
				{
					size_t newCount = count + 1;
					size_t newMemPoolIndex = _GetMemPoolIndex(newCount);
					Memory memory(params.GetMemPool(newMemPoolIndex));
					Item* newItems = _GetItems(memory.GetPointer());
					ItemTraits::RelocateCreate(_GetItems(), newItems, count,
						itemCreator, newItems + count);
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

		void DecCount(Params& params) MOMO_NOEXCEPT
		{
			size_t count = _GetCount();
			MOMO_ASSERT(count > 0);
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
			MOMO_ASSERT(!_IsEmpty());
			return reinterpret_cast<unsigned char*>(mPtr);
		}

		void _Set(unsigned char* ptr, size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(ptr != nullptr);
			mPtr = reinterpret_cast<uintptr_t>(ptr);
			*ptr = (unsigned char)((memPoolIndex << 4) | count);
		}

		static size_t _GetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(0 < count && count <= maxCount);
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
			return reinterpret_cast<Item*>(ptr + ItemTraits::alignment);
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

	template<typename TItemTraits, size_t tMaxCount, typename TMemPoolParams>
	class BucketLimP<TItemTraits, tMaxCount, TMemPoolParams, true>
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount < 16);

		static const bool useUIntPtr = true;

		typedef BucketBounds<Item> Bounds;
		typedef typename Bounds::ConstBounds ConstBounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef momo::MemPool<MemPoolParams, MemManagerPtr> MemPool;

		typedef BucketMemory<MemPool, Item*> Memory;

		static const size_t minItemAlignment = (maxCount <= 1) ? 1 : (maxCount <= 2) ? 2
			: (maxCount <= 4) ? 4 : (maxCount <= 8) ? 8 : 16;
		static const size_t itemAlignment = (ItemTraits::alignment < minItemAlignment)
			? minItemAlignment : ItemTraits::alignment;

		static const bool skipOddMemPools = (maxCount > 1 && sizeof(Item) <= itemAlignment);	//?
		static const uintptr_t modMemPoolIndex =
			(uintptr_t)minItemAlignment / (skipOddMemPools ? 2 : 1);

		static const uintptr_t stateNull = UIntPtrConst::null;
		static const uintptr_t stateNullWasFull = UIntPtrConst::invalid;

	public:
		class Params
		{
		private:
			typedef ArrayIntCap<maxCount, MemPool, MemManagerDummy> MemPools;

		public:
			explicit Params(MemManager& memManager)
			{
				for (size_t i = 1; i <= maxCount + (skipOddMemPools ? 1 : 0); ++i)
				{
					if (skipOddMemPools && i % 2 == 1)
						continue;
					size_t blockAlignment = internal::UIntMath<size_t>::Ceil(
						i * (size_t)modMemPoolIndex, itemAlignment);
					mMemPools.AddBackNogrow(MemPool(MemPoolParams(i * sizeof(Item), blockAlignment),
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
				MOMO_ASSERT(memPoolIndex > 0);
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
			MOMO_ASSERT(_IsEmpty());
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
				MOMO_ASSERT(count <= memPoolIndex);
				MOMO_ASSERT(count < maxCount);
				if (count == memPoolIndex)
				{
					size_t newCount = count + 1;
					size_t newMemPoolIndex = _GetMemPoolIndex(newCount);
					Memory memory(params.GetMemPool(newMemPoolIndex));
					Item* newItems = memory.GetPointer();
					ItemTraits::RelocateCreate(items, newItems, count,
						itemCreator, newItems + count);
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

		void DecCount(Params& params) MOMO_NOEXCEPT
		{
			Bounds bounds = _GetBounds();
			size_t count = bounds.GetCount();
			MOMO_ASSERT(count > 0);
			if (count == 1)
			{
				size_t memPoolIndex = _GetMemPoolIndex();
				params.GetMemPool(memPoolIndex).Deallocate(bounds.GetBegin());
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
			mPtrState = reinterpret_cast<uintptr_t>(items) + (uintptr_t)(count - 1) * modMemPoolIndex
				+ (uintptr_t)memPoolIndex / (skipOddMemPools ? 2 : 1) - 1;
		}

		static size_t _GetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(0 < count && count <= maxCount);
			return count + (skipOddMemPools ? count % 2 : 0);
		}

		size_t _GetMemPoolIndex() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!_IsEmpty());
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
			MOMO_ASSERT(count1 < memPoolIndex);
			Item* items = reinterpret_cast<Item*>((ptrCount - count1) * modMemPoolIndex);
			return Bounds(items, (size_t)count1 + 1);
		}

	private:
		uintptr_t mPtrState;
	};
}

template<size_t tMaxCount = sizeof(void*),
	typename TMemPoolParams = MemPoolParams<>,
	bool tUseUIntPtr = true>
struct HashBucketLimP : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;
	static const bool useUIntPtr = tUseUIntPtr;

	typedef TMemPoolParams MemPoolParams;

private:
	template<typename ItemTraits>
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
		typedef internal::BucketLimP<ItemTraits, maxCount, MemPoolParams, useUIntPtr> Bucket;
	};

public:
	template<typename ItemTraits>
	using Bucket = typename Bucketer<ItemTraits>::Bucket;
};

} // namespace momo

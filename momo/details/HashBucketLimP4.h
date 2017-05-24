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
	struct BucketLimP4PointerSelector;

	template<typename Item>
	struct BucketLimP4PointerSelector<Item, 4>
	{
		typedef Item* Pointer;
	};

	template<typename Item>
	struct BucketLimP4PointerSelector<Item, 8>
	{
		class Pointer
		{
		public:
			Pointer& operator=(Item* ptr) MOMO_NOEXCEPT
			{
				uint64_t intPtr = reinterpret_cast<uint64_t>(ptr);
				mFirst = (uint32_t)intPtr;
				mSecond = (uint32_t)(intPtr >> 32);
				return *this;
			}

			operator Item*() const MOMO_NOEXCEPT
			{
				uint64_t intPtr = ((uint64_t)mSecond << 32) | (uint64_t)mFirst;
				return reinterpret_cast<Item*>(intPtr);
			}

		private:
			uint32_t mFirst;
			uint32_t mSecond;
		};
	};

	template<typename TItemTraits, size_t tMaxCount, typename TMemPoolParams>
	class BucketLimP4
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;

		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount < 4);

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef BucketBounds<Item> Bounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef momo::MemPool<MemPoolParams, MemManagerPtr, NestedMemPoolSettings> MemPool;

		typedef BucketMemory<MemPool, Item*> Memory;

		typedef typename BucketLimP4PointerSelector<Item>::Pointer Pointer;

	public:
		class Params
		{
		public:
			static const bool skipFirstMemPool =
				(maxCount > 1 && ItemTraits::alignment == sizeof(Item));	//?

		private:
			typedef NestedArrayIntCap<maxCount, MemPool, MemManagerDummy> MemPools;

			static const size_t minMemPoolIndex = skipFirstMemPool ? 2 : 1;

		public:
			explicit Params(MemManager& memManager)
			{
				for (size_t i = minMemPoolIndex; i <= maxCount; ++i)
				{
					size_t blockSize = i * sizeof(Item);
					mMemPools.AddBackNogrow(MemPool(MemPoolParams(blockSize, ItemTraits::alignment),
						MemManagerPtr(memManager)));
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

			MemPool& GetMemPool(size_t memPoolIndex) MOMO_NOEXCEPT
			{
				MOMO_ASSERT(memPoolIndex >= minMemPoolIndex);
				return mMemPools[memPoolIndex - minMemPoolIndex];
			}

		private:
			MemPools mMemPools;
		};

	public:
		BucketLimP4() MOMO_NOEXCEPT
		{
			pvSetState(nullptr, pvGetMemPoolIndex(1), 0);
		}

		BucketLimP4(const BucketLimP4&) = delete;

		~BucketLimP4() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(pvGetItems() == nullptr);
		}

		BucketLimP4& operator=(const BucketLimP4&) = delete;

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return Bounds(pvGetItems(), pvGetCount());
		}

		bool TestIndex(size_t index, size_t hashCode) const MOMO_NOEXCEPT
		{
			return hashCode >> (sizeof(size_t) * 8 - 7)
				== (size_t)((mCodeState >> (index * 7)) & 127);
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return pvGetCount() == maxCount;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return pvGetMemPoolIndex() == pvGetMemPoolIndex(maxCount);
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			Item* items = pvGetItems();
			if (items != nullptr)
			{
				ItemTraits::Destroy(params.GetMemManager(), items, pvGetCount());
				params.GetMemPool(pvGetMemPoolIndex()).Deallocate(items);
			}
			pvSetState(nullptr, pvGetMemPoolIndex(1), 0);
		}

		template<typename ItemCreator>
		Item* AddBackCrt(Params& params, const ItemCreator& itemCreator, size_t hashCode)
		{
			Item* items = pvGetItems();
			if (items == nullptr)
			{
				size_t newCount = 1;
				size_t newMemPoolIndex = pvGetMemPoolIndex();
				Memory memory(params.GetMemPool(newMemPoolIndex));
				Item* newItems = memory.GetPointer();
				itemCreator(newItems);
				pvSetState(memory.Extract(), newMemPoolIndex, newCount);
				pvSetCode(0, hashCode);
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
					Item* newItems = memory.GetPointer();
					ItemTraits::RelocateCreate(params.GetMemManager(), items, newItems, count,
						itemCreator, newItems + count);
					params.GetMemPool(memPoolIndex).Deallocate(items);
					pvSetState(memory.Extract(), newMemPoolIndex, newCount);
					pvSetCode(count, hashCode);
					return newItems + count;
				}
				else
				{
					itemCreator(items + count);
					mCodeState += (uint32_t)1 << 28;
					pvSetCode(count, hashCode);
					return items + count;
				}
			}
		}

		void AcceptRemove(Params& params, size_t index) MOMO_NOEXCEPT
		{
			size_t count = pvGetCount();
			MOMO_ASSERT(count > 0);
			if (count == 1)
			{
				MOMO_ASSERT(index == 0);
				size_t memPoolIndex = pvGetMemPoolIndex();
				params.GetMemPool(memPoolIndex).Deallocate(pvGetItems());
				if (memPoolIndex != pvGetMemPoolIndex(maxCount))
					memPoolIndex = pvGetMemPoolIndex(1);
				pvSetState(nullptr, memPoolIndex, 0);
			}
			else
			{
				MOMO_ASSERT(index < count);
				size_t hashCode = (size_t)((mCodeState >> (count * 7 - 7)) & 127);
				hashCode <<= sizeof(size_t) * 8 - 7;
				pvSetCode(index, hashCode);
				mCodeState -= (uint32_t)1 << 28;
			}
		}

	private:
		void pvSetState(Item* items, size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			mItemPtr = items;
			mCodeState &= ~((uint32_t)15 << 28);
			mCodeState |= (uint32_t)((memPoolIndex << 2) | count) << 28;
		}

		static size_t pvGetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(0 < count && count <= maxCount);
			if (Params::skipFirstMemPool && count == 1)
				return 2;
			return count;
		}

		size_t pvGetMemPoolIndex() const MOMO_NOEXCEPT
		{
			return (size_t)(mCodeState >> 30);
		}

		size_t pvGetCount() const MOMO_NOEXCEPT
		{
			return (size_t)((mCodeState >> 28) & 3);
		}

		void pvSetCode(size_t index, size_t hashCode) MOMO_NOEXCEPT
		{
			mCodeState &= ~((uint32_t)127 << (index * 7));
			mCodeState |= (uint32_t)(hashCode >> (sizeof(size_t) * 8 - 7) << (index * 7));
		}

		Item* pvGetItems() const MOMO_NOEXCEPT
		{
			return mItemPtr;
		}

	private:
		Pointer mItemPtr;
		uint32_t mCodeState;
	};
}

template<size_t tMaxCount = 3,
	typename TMemPoolParams = MemPoolParams<>>
struct HashBucketLimP4 : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;

	typedef TMemPoolParams MemPoolParams;

	template<typename ItemTraits>
	using Bucket = internal::BucketLimP4<ItemTraits, maxCount, MemPoolParams>;
};

} // namespace momo

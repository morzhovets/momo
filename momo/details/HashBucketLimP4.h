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
				mIntPtr1 = (uint32_t)intPtr;
				mIntPtr2 = (uint32_t)(intPtr >> 32);
				return *this;
			}

			operator Item*() const MOMO_NOEXCEPT
			{
				uint64_t intPtr = ((uint64_t)mIntPtr2 << 32) | (uint64_t)mIntPtr1;
				return reinterpret_cast<Item*>(intPtr);
			}

		private:
			uint32_t mIntPtr1;
			uint32_t mIntPtr2;
		};
	};

	template<typename TItemTraits, size_t tMaxCount, typename TMemPoolParams>
	class BucketLimP4
	{
	protected:
		typedef TItemTraits ItemTraits;
		typedef TMemPoolParams MemPoolParams;

		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount <= 4);

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef Item* Iterator;
		typedef BucketBounds<Iterator> Bounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		static const size_t minMemPoolIndex =
			(maxCount > 1 && ItemTraits::alignment == sizeof(Item)) ? 2 : 1;

		template<size_t memPoolIndex>
		using MemPoolParamsStatic = momo::MemPoolParamsStatic<memPoolIndex * sizeof(Item),
			ItemTraits::alignment, MemPoolParams::blockCount,
			(/*minMemPoolIndex <= memPoolIndex &&*/ memPoolIndex <= maxCount)	// vs2013
				? MemPoolParams::cachedFreeBlockCount : 0>;

		template<size_t memPoolIndex>
		using MemPool = momo::MemPool<MemPoolParamsStatic<memPoolIndex>, MemManagerPtr,
			NestedMemPoolSettings>;

		template<size_t memPoolIndex>
		using Memory = BucketMemory<MemPool<memPoolIndex>, Item*>;

		typedef typename BucketLimP4PointerSelector<Item>::Pointer Pointer;

		static const uint8_t emptyHash = 240;

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
			pvSetState0(minMemPoolIndex);
		}

		BucketLimP4(const BucketLimP4&) = delete;

		~BucketLimP4() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mItemPtr == nullptr);
		}

		BucketLimP4& operator=(const BucketLimP4&) = delete;

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return Bounds(mItemPtr, pvGetCount());
		}

		template<typename Predicate>
		Iterator Find(Params& /*params*/, const Predicate& pred, size_t hashCode,
			size_t /*logBucketCount*/)
		{
			uint8_t hashByte = pvGetHashByte(hashCode);
			Item* items = mItemPtr;
			for (size_t i = 0; i < maxCount; ++i)
			{
				if (mHashesState[i] == hashByte && pred(items[i]))
					return items + i;
			}
			return nullptr;
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return pvGetCount() == maxCount;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return pvGetMemPoolIndex() == maxCount;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			Item* items = mItemPtr;
			if (items != nullptr)
			{
				ItemTraits::Destroy(params.GetMemManager(), items, pvGetCount());
				pvDeallocate(params, pvGetMemPoolIndex(), items);
			}
			pvSetState0(minMemPoolIndex);
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& params, const ItemCreator& itemCreator, size_t hashCode,
			size_t /*logBucketCount*/)
		{
			Item* items = mItemPtr;
			size_t memPoolIndex = pvGetMemPoolIndex();
			if (items == nullptr)
			{
				MOMO_ASSERT(memPoolIndex == minMemPoolIndex || memPoolIndex == maxCount);
				if (memPoolIndex == minMemPoolIndex)
					return pvAdd0<minMemPoolIndex>(params, itemCreator, hashCode);
				else
					return pvAdd0<maxCount>(params, itemCreator, hashCode);
			}
			else
			{
				size_t count = pvGetCount();
				MOMO_ASSERT(count <= memPoolIndex);
				MOMO_ASSERT(count < maxCount);
				if (count == memPoolIndex)
				{
					switch (memPoolIndex)
					{
					case 1:
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
					itemCreator(items + count);
					mHashesState[count] = pvGetHashByte(hashCode);
					pvSetState(items, memPoolIndex, count + 1);
					return items + count;
				}
			}
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& params, Iterator iter, const ItemReplacer& itemReplacer)
		{
			Item* items = mItemPtr;
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
				pvSetState0(memPoolIndex);
				return nullptr;
			}
			else
			{
				size_t index = iter - items;
				MOMO_ASSERT(index < count);
				itemReplacer(items[count - 1], *iter);
				mHashesState[index] = mHashesState[count - 1];
				mHashesState[count - 1] = emptyHash;
				pvSetState(items, memPoolIndex, count - 1);
				return iter;
			}
		}

	private:
		void pvSetState0(size_t memPoolIndex) MOMO_NOEXCEPT
		{
			std::fill_n(mHashesState, 4, (uint8_t)emptyHash);
			pvSetState(nullptr, memPoolIndex, 0);
		}

		void pvSetState(Item* items, size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			mItemPtr = items;
			if (maxCount < 4 || count < 4)
				mHashesState[3] = emptyHash + (uint8_t)((count << 2) | (memPoolIndex - 1));
		}

		size_t pvGetMemPoolIndex() const MOMO_NOEXCEPT
		{
			if (maxCount == 4 && mHashesState[3] < emptyHash)
				return 4;
			return (size_t)(mHashesState[3] & 3) + 1;
		}

		size_t pvGetCount() const MOMO_NOEXCEPT
		{
			if (maxCount == 4 && mHashesState[3] < emptyHash)
				return 4;
			return (size_t)((mHashesState[3] >> 2) & 3);
		}

		static uint8_t pvGetHashByte(size_t hashCode) MOMO_NOEXCEPT
		{
			uint32_t hashCode24 = (uint32_t)(hashCode >> (sizeof(size_t) * 8 - 24));
			return (uint8_t)((hashCode24 * (uint32_t)emptyHash) >> 24);
		}

		template<size_t memPoolIndex, typename ItemCreator>
		Item* pvAdd0(Params& params, const ItemCreator& itemCreator, size_t hashCode)
		{
			Memory<memPoolIndex> memory(params.template GetMemPool<memPoolIndex>());
			Item* items = memory.GetPointer();
			itemCreator(items);
			mHashesState[0] = pvGetHashByte(hashCode);
			pvSetState(memory.Extract(), memPoolIndex, 1);
			return items;
		}

		template<size_t memPoolIndex, typename ItemCreator>
		Item* pvAdd(Params& params, const ItemCreator& itemCreator, size_t hashCode,
			Item* items)
		{
			static const size_t newMemPoolIndex = memPoolIndex + 1;
			size_t count = memPoolIndex;
			size_t newCount = count + 1;
			Memory<newMemPoolIndex> memory(params.template GetMemPool<newMemPoolIndex>());
			Item* newItems = memory.GetPointer();
			ItemTraits::RelocateCreate(params.GetMemManager(), items, newItems, count,
				itemCreator, newItems + count);
			params.template GetMemPool<memPoolIndex>().Deallocate(items);
			mHashesState[count] = pvGetHashByte(hashCode);
			pvSetState(memory.Extract(), newMemPoolIndex, newCount);
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
		Pointer mItemPtr;
		uint8_t mHashesState[4];
	};
}

template<size_t tMaxCount = 4,
	typename TMemPoolParams = MemPoolParams<>>
struct HashBucketLimP4 : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;

	typedef TMemPoolParams MemPoolParams;

	template<typename ItemTraits>
	using Bucket = internal::BucketLimP4<ItemTraits, maxCount, MemPoolParams>;
};

} // namespace momo

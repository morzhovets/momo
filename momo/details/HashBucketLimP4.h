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
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount <= 4);

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef BucketBounds<Item> Bounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		template<size_t memPoolIndex>
		using MemPoolParamsStatic = momo::MemPoolParamsStatic<memPoolIndex * sizeof(Item),
			ItemTraits::alignment, MemPoolParams::blockCount, MemPoolParams::cachedFreeBlockCount>;

		template<size_t memPoolIndex>
		using MemPool = momo::MemPool<MemPoolParamsStatic<memPoolIndex>, MemManagerPtr,
			NestedMemPoolSettings>;

		template<size_t memPoolIndex>
		using Memory = BucketMemory<MemPool<memPoolIndex>, Item*>;

		typedef typename BucketLimP4PointerSelector<Item>::Pointer Pointer;

	public:
		class Params
		{
		public:
			static const bool skipFirstMemPool = false;
				//(maxCount > 1 && ItemTraits::alignment == sizeof(Item));	//?

		private:
			typedef std::tuple<MemPool<1>, MemPool<2>, MemPool<3>, MemPool<4>> MemPools;

			//static const size_t minMemPoolIndex = skipFirstMemPool ? 2 : 1;

		public:
			explicit Params(MemManager& memManager)
				: mMemPools(MemManagerPtr(memManager), MemManagerPtr(memManager),
					MemManagerPtr(memManager), MemManagerPtr(memManager))
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
			MOMO_ASSERT(mItemPtr == nullptr);
		}

		BucketLimP4& operator=(const BucketLimP4&) = delete;

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			Item* items = mItemPtr;
			size_t count = pvGetCount() - ((items == nullptr) ? 1 : 0);
			return Bounds(items, count);
		}

		bool TestIndex(size_t index, size_t hashCode) const MOMO_NOEXCEPT
		{
			return hashCode >> (sizeof(size_t) * 8 - 7)
				== (size_t)((mCodeState >> (index * 7)) & 127);
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			if (maxCount == 1 && mItemPtr == nullptr)
				return false;
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
			pvSetState0(1);
		}

		template<typename ItemCreator>
		Item* AddBackCrt(Params& params, const ItemCreator& itemCreator, size_t hashCode)
		{
			Item* items = mItemPtr;
			if (items == nullptr)
			{
				if (pvGetMemPoolIndex() == 1)
					return pvAddBack0<1>(params, itemCreator, hashCode);
				else
					return pvAddBack0<maxCount>(params, itemCreator, hashCode);
			}
			else
			{
				size_t memPoolIndex = pvGetMemPoolIndex();
				size_t count = pvGetCount();
				MOMO_ASSERT(count <= memPoolIndex);
				MOMO_ASSERT(count < maxCount);
				if (count == memPoolIndex)
				{
					switch (memPoolIndex)
					{
					case 1:
						return pvAddBack<1>(params, itemCreator, hashCode, items);
					case 2:
						return pvAddBack<2>(params, itemCreator, hashCode, items);
					default:
						MOMO_ASSERT(memPoolIndex == 3);
						return pvAddBack<3>(params, itemCreator, hashCode, items);
					}
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
			Item* items = mItemPtr;
			MOMO_ASSERT(items != nullptr);
			size_t count = pvGetCount();
			if (count == 1)
			{
				MOMO_ASSERT(index == 0);
				size_t memPoolIndex = pvGetMemPoolIndex();
				pvDeallocate(params, memPoolIndex, items);
				if (memPoolIndex != maxCount)
					memPoolIndex = 1;
				pvSetState0(memPoolIndex);
			}
			else
			{
				MOMO_ASSERT(index < count);
				mCodeState -= (uint32_t)1 << 28;
				size_t hashCode = (size_t)((mCodeState >> (count * 7 - 7)) & 127);
				hashCode <<= sizeof(size_t) * 8 - 7;
				pvSetCode(index, hashCode);
			}
		}

	private:
		void pvSetState0(size_t memPoolIndex) MOMO_NOEXCEPT
		{
			pvSetState(nullptr, memPoolIndex, 1);
		}

		void pvSetState(Item* items, size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			mItemPtr = items;
			mCodeState &= ((uint32_t)1 << 28) - 1;
			mCodeState |= (uint32_t)(((memPoolIndex - 1) << 2) | (count - 1)) << 28;
		}

		//static size_t pvGetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		//{
		//	MOMO_ASSERT(0 < count && count <= maxCount);
		//	if (Params::skipFirstMemPool && count == 1)
		//		return 2;
		//	return count;
		//}

		size_t pvGetMemPoolIndex() const MOMO_NOEXCEPT
		{
			return (size_t)(mCodeState >> 30) + 1;
		}

		size_t pvGetCount() const MOMO_NOEXCEPT
		{
			return (size_t)((mCodeState >> 28) & 3) + 1;
		}

		void pvSetCode(size_t index, size_t hashCode) MOMO_NOEXCEPT
		{
			mCodeState &= ~((uint32_t)127 << (index * 7));
			mCodeState |= (uint32_t)(hashCode >> (sizeof(size_t) * 8 - 7) << (index * 7));
		}

		template<size_t memPoolIndex, typename ItemCreator>
		Item* pvAddBack0(Params& params, const ItemCreator& itemCreator, size_t hashCode)
		{
			Memory<memPoolIndex> memory(params.GetMemPool<memPoolIndex>());
			Item* items = memory.GetPointer();
			itemCreator(items);
			pvSetState(memory.Extract(), memPoolIndex, 1);
			pvSetCode(0, hashCode);
			return items;
		}

		template<size_t memPoolIndex, typename ItemCreator>
		Item* pvAddBack(Params& params, const ItemCreator& itemCreator, size_t hashCode, Item* items)
		{
			static const size_t newMemPoolIndex = memPoolIndex + 1;
			size_t count = memPoolIndex;
			size_t newCount = count + 1;
			Memory<newMemPoolIndex> memory(params.GetMemPool<newMemPoolIndex>());
			Item* newItems = memory.GetPointer();
			ItemTraits::RelocateCreate(params.GetMemManager(), items, newItems, count,
				itemCreator, newItems + count);
			params.GetMemPool<memPoolIndex>().Deallocate(items);
			pvSetState(memory.Extract(), newMemPoolIndex, newCount);
			pvSetCode(count, hashCode);
			return newItems + count;
		}

		void pvDeallocate(Params& params, size_t memPoolIndex, Item* items) MOMO_NOEXCEPT
		{
			switch (memPoolIndex)
			{
			case 1:
				params.GetMemPool<1>().Deallocate(items);
				break;
			case 2:
				params.GetMemPool<2>().Deallocate(items);
				break;
			case 3:
				params.GetMemPool<3>().Deallocate(items);
				break;
			case 4:
				params.GetMemPool<4>().Deallocate(items);
				break;
			default:
				MOMO_ASSERT(false);
			}
		}

	private:
		Pointer mItemPtr;
		uint32_t mCodeState;
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

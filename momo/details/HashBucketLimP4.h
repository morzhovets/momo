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
	class BucketLimP4Pointer;

	template<typename Item>
	class BucketLimP4Pointer<Item, 4>
	{
	public:
		void Set(Item* ptr, bool lastBit) MOMO_NOEXCEPT
		{
			mIntPtr = reinterpret_cast<uint32_t>(ptr);
			MOMO_ASSERT((mIntPtr & 1) == 0);
			mIntPtr |= (uint32_t)(lastBit ? 1 : 0);
		}

		Item* GetPointer() const MOMO_NOEXCEPT
		{
			uint32_t intPtr = mIntPtr & ~(uint32_t)1;
			return reinterpret_cast<Item*>(intPtr);
		}

		bool GetLastBit() const MOMO_NOEXCEPT
		{
			return (mIntPtr & 1) != 0;
		}

	private:
		uint32_t mIntPtr;
	};

	template<typename Item>
	class BucketLimP4Pointer<Item, 8>
	{
	public:
		void Set(Item* ptr, bool lastBit) MOMO_NOEXCEPT
		{
			uint64_t intPtr = reinterpret_cast<uint64_t>(ptr);
			MOMO_ASSERT((intPtr & 1) == 0);
			mIntPtr1 = (uint32_t)intPtr | (uint32_t)(lastBit ? 1 : 0);
			mIntPtr2 = (uint32_t)(intPtr >> 32);
		}

		Item* GetPointer() const MOMO_NOEXCEPT
		{
			uint64_t intPtr = ((uint64_t)mIntPtr2 << 32) | (uint64_t)mIntPtr1;
			intPtr &= ~(uint64_t)1;
			return reinterpret_cast<Item*>(intPtr);
		}

		bool GetLastBit() const MOMO_NOEXCEPT
		{
			return (mIntPtr1 & 1) != 0;
		}

	private:
		uint32_t mIntPtr1;
		uint32_t mIntPtr2;
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

		static const size_t itemAlignment = (ItemTraits::alignment > 1) ? ItemTraits::alignment : 2;

		template<size_t memPoolIndex>
		using MemPoolParamsStatic = momo::MemPoolParamsStatic<memPoolIndex * sizeof(Item),
			itemAlignment, MemPoolParams::blockCount,
			(memPoolIndex <= maxCount) ? MemPoolParams::cachedFreeBlockCount : 0>;

		template<size_t memPoolIndex>
		using MemPool = momo::MemPool<MemPoolParamsStatic<memPoolIndex>, MemManagerPtr,
			NestedMemPoolSettings>;

		template<size_t memPoolIndex>
		using Memory = BucketMemory<MemPool<memPoolIndex>, Item*>;

		typedef BucketLimP4Pointer<Item> Pointer;

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
			MOMO_ASSERT(mItemPtr.GetPointer() == nullptr);
		}

		BucketLimP4& operator=(const BucketLimP4&) = delete;

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return Bounds(mItemPtr.GetPointer(), pvGetCount());
		}

		template<typename Predicate>
		const Item* Find(Params& /*params*/, const Predicate& pred, size_t hashCode) const
		{
			uint8_t hashByte = pvGetHashByte(hashCode);
			const Item* items = mItemPtr.GetPointer();
			size_t count = pvGetCount();
			for (size_t i = 0; i < count; ++i)
			{
				if (mHashesState[i] == hashByte && pred(items[i]))
					return items + i;
			}
			return nullptr;
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			if (maxCount == 1 && mItemPtr.GetPointer() == nullptr)
				return false;
			return pvGetCount() == maxCount;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return pvGetMemPoolIndex() == maxCount;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			Item* items = mItemPtr.GetPointer();
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
			Item* items = mItemPtr.GetPointer();
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
					pvSetHash(count, hashCode);
					pvSetState(items, memPoolIndex, count + 1);
					return items + count;
				}
			}
		}

		void AcceptRemove(Params& params, size_t index) MOMO_NOEXCEPT
		{
			Item* items = mItemPtr.GetPointer();
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
				size_t hashCode = (size_t)mHashesState[count - 1] << (sizeof(size_t) * 8 - 8);
				pvSetHash(index, hashCode);
				pvSetState(items, pvGetMemPoolIndex(), count - 1);
			}
		}

	private:
		void pvSetState0(size_t memPoolIndex) MOMO_NOEXCEPT
		{
			pvSetState(nullptr, memPoolIndex, 0);
		}

		void pvSetState(Item* items, size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			if (maxCount < 4 || count < 4)
			{
				mItemPtr.Set(items, false);
				mHashesState[3] = (uint8_t)(((memPoolIndex - 1) << 2) | count);
			}
			else
			{
				mItemPtr.Set(items, true);
			}
		}

		size_t pvGetMemPoolIndex() const MOMO_NOEXCEPT
		{
			if (maxCount == 4 && mItemPtr.GetLastBit())
				return 4;
			return (size_t)(mHashesState[3] >> 2) + 1;
		}

		size_t pvGetCount() const MOMO_NOEXCEPT
		{
			if (maxCount == 4 && mItemPtr.GetLastBit())
				return 4;
			return (size_t)(mHashesState[3] & 3);
		}

		static uint8_t pvGetHashByte(size_t hashCode) MOMO_NOEXCEPT
		{
			return (uint8_t)(hashCode >> (sizeof(size_t) * 8 - 8));
		}

		void pvSetHash(size_t index, size_t hashCode) MOMO_NOEXCEPT
		{
			mHashesState[index] = pvGetHashByte(hashCode);
		}

		template<size_t memPoolIndex, typename ItemCreator>
		Item* pvAddBack0(Params& params, const ItemCreator& itemCreator, size_t hashCode)
		{
			Memory<memPoolIndex> memory(params.template GetMemPool<memPoolIndex>());
			Item* items = memory.GetPointer();
			itemCreator(items);
			pvSetState(memory.Extract(), memPoolIndex, 1);
			pvSetHash(0, hashCode);
			return items;
		}

		template<size_t memPoolIndex, typename ItemCreator>
		Item* pvAddBack(Params& params, const ItemCreator& itemCreator, size_t hashCode,
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
			pvSetHash(count, hashCode);
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

/**********************************************************\

  momo/HashBuckets/BucketLimP1.h

  namespace momo:
    struct HashBucketLimP1

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../MemPool.h"
#include "../Array.h"

namespace momo
{

namespace internal
{
#pragma pack(push, 1)
	template<typename TItemTraits, typename TMemManager,
		size_t tMaxCount, size_t tMemPoolBlockCount>
	class BucketLimP1
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemManager MemManager;
		typedef typename ItemTraits::Item Item;

		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount < 16);

		static const size_t memPoolBlockCount = tMemPoolBlockCount;

		typedef BucketBounds<Item> Bounds;
		typedef typename Bounds::ConstBounds ConstBounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;
		typedef momo::MemPool<memPoolBlockCount, MemManagerPtr> MemPool;

		typedef BucketMemory<MemPool, Item*> Memory;

	public:
		class Params
		{
		private:
			typedef momo::Array<MemPool, MemManagerPtr> MemPools;

		public:
			Params(MemManager& memManager)
				: mMemPools(MemManagerPtr(memManager))
			{
				mMemPools.Reserve(maxCount);
				for (size_t i = 1; i <= maxCount; ++i)
				{
					mMemPools.AddBackNogrow(MemPool(i * sizeof(Item),
						MemManagerPtr(memManager)));
				}
			}

			MemPool& operator[](size_t index) MOMO_NOEXCEPT
			{
				assert(index > 0);
				return mMemPools[index - 1];
			}

		private:
			MOMO_DISABLE_COPY_CONSTRUCTOR(Params);
			MOMO_DISABLE_COPY_OPERATOR(Params);

		private:
			MemPools mMemPools;
		};

	public:
		BucketLimP1() MOMO_NOEXCEPT
			: mItems(nullptr),
			mState(0)
		{
		}

		~BucketLimP1() MOMO_NOEXCEPT
		{
			assert(mItems == nullptr);
		}

		ConstBounds GetBounds(const Params& /*params*/) const MOMO_NOEXCEPT
		{
			return ConstBounds(mItems, _GetCount());
		}

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return Bounds(mItems, _GetCount());
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return _GetCount() == maxCount;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return _GetMemPoolIndex() == maxCount;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (mItems != nullptr)
			{
				ItemTraits::Destroy(mItems, _GetCount());
				params[_GetMemPoolIndex()].FreeMemory(mItems);
			}
			_Set(nullptr, (unsigned char)0);
		}

		template<typename ItemCreator>
		void AddBackEmpl(Params& params, const ItemCreator& itemCreator)
		{
			if (mItems == nullptr)
			{
				size_t newCount = 1;
				size_t newMemPoolIndex = _GetMemPoolIndex(newCount);
				Memory memory(params[newMemPoolIndex]);
				itemCreator(memory.GetPointer());
				_Set(memory.Extract(), _MakeState(newMemPoolIndex, newCount));
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
					Memory memory(params[newMemPoolIndex]);
					ItemTraits::RelocateAddBack(mItems,
						memory.GetPointer(), count, itemCreator);
					params[memPoolIndex].FreeMemory(mItems);
					_Set(memory.Extract(), _MakeState(newMemPoolIndex, newCount));
				}
				else
				{
					itemCreator(mItems + count);
					++mState;
				}
			}
		}

		void RemoveBack(Params& params) MOMO_NOEXCEPT
		{
			size_t count = _GetCount();
			assert(count > 0);
			ItemTraits::Destroy(mItems + count - 1, 1);
			if (count == 1 && !WasFull())
			{
				params[_GetMemPoolIndex()].FreeMemory(mItems);
				_Set(nullptr, (unsigned char)0);
			}
			else
			{
				--mState;
			}
		}

	private:
		void _Set(Item* items, unsigned char state) MOMO_NOEXCEPT
		{
			mItems = items;
			mState = state;
		}

		static unsigned char _MakeState(size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			return (unsigned char)((memPoolIndex << 4) | count);
		}

		static size_t _GetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			assert(0 < count && count <= maxCount);
			return count;
		}

		size_t _GetMemPoolIndex() const MOMO_NOEXCEPT
		{
			return (size_t)(mState >> 4);
		}

		size_t _GetCount() const MOMO_NOEXCEPT
		{
			return (size_t)(mState & 15);
		}

	private:
		MOMO_DISABLE_COPY_CONSTRUCTOR(BucketLimP1);
		MOMO_DISABLE_COPY_OPERATOR(BucketLimP1);

	private:
		Item* mItems;
		unsigned char mState;
	};
#pragma pack(pop)
}

template<size_t tMaxCount = 7,
	size_t tMemPoolBlockCount = 32>
struct HashBucketLimP1
{
	static const size_t maxCount = tMaxCount;
	static const size_t memPoolBlockCount = tMemPoolBlockCount;

	static const size_t logStartBucketCount = 4;

	static size_t CalcCapacity(size_t bucketCount) MOMO_NOEXCEPT
	{
		return (size_t)((double)bucketCount * 1.46);
	}

	static size_t GetBucketCountShift(size_t bucketCount) MOMO_NOEXCEPT
	{
		return bucketCount < (1 << 20) ? 2 : 1;
	}

	template<typename ItemTraits, typename MemManager>
	struct Bucketer
	{
		typedef internal::BucketLimP1<ItemTraits, MemManager,
			maxCount, memPoolBlockCount> Bucket;
	};
};

} // namespace momo

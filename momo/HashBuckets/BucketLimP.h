/**********************************************************\

  momo/HashBuckets/BucketLimP.h

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
		size_t tMaxCount, size_t tMemPoolBlockCount>
	class BucketLimP
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

		typedef BucketMemory<MemPool, unsigned char*> Memory;

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
					mMemPools.AddBackNogrow(MemPool(i * sizeof(Item) + ItemTraits::alignment,
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
		BucketLimP() MOMO_NOEXCEPT
			: mPtr(nullptr)
		{
		}

		~BucketLimP() MOMO_NOEXCEPT
		{
			assert(mPtr == nullptr);
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
			if (mPtr == nullptr)
				return false;
			return _GetCount() == maxCount;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			if (mPtr == nullptr)
				return false;
			return _GetMemPoolIndex() == maxCount;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (mPtr == nullptr)
				return;
			ItemTraits::Destroy(_GetItems(), _GetCount());
			params[_GetMemPoolIndex()].Deallocate(mPtr);
			mPtr = nullptr;
		}

		template<typename ItemCreator>
		void AddBackEmpl(Params& params, const ItemCreator& itemCreator)
		{
			if (mPtr == nullptr)
			{
				size_t newCount = 1;
				size_t newMemPoolIndex = _GetMemPoolIndex(newCount);
				Memory memory(params[newMemPoolIndex]);
				itemCreator(_GetItems(memory.GetPointer()));
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
					Memory memory(params[newMemPoolIndex]);
					ItemTraits::RelocateAddBack(_GetItems(), _GetItems(memory.GetPointer()),
						count, itemCreator);
					params[memPoolIndex].Deallocate(mPtr);
					_Set(memory.Extract(), newMemPoolIndex, newCount);
				}
				else
				{
					itemCreator(_GetItems() + count);
					++*mPtr;
				}
			}
		}

		void RemoveBack(Params& params) MOMO_NOEXCEPT
		{
			size_t count = _GetCount();
			assert(count > 0);
			ItemTraits::Destroy(_GetItems() + count - 1, 1);
			if (count == 1 && !WasFull())
			{
				params[_GetMemPoolIndex()].Deallocate(mPtr);
				mPtr = nullptr;
			}
			else
			{
				--*mPtr;
			}
		}

	private:
		void _Set(unsigned char* ptr, size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			assert(ptr != nullptr);
			mPtr = ptr;
			*mPtr = (unsigned char)((memPoolIndex << 4) | count);
		}

		static size_t _GetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			assert(0 < count && count <= maxCount);
			return count;
		}

		size_t _GetMemPoolIndex() const MOMO_NOEXCEPT
		{
			assert(mPtr != nullptr);
			return (size_t)(*mPtr >> 4);
		}

		size_t _GetCount() const MOMO_NOEXCEPT
		{
			assert(_GetMemPoolIndex() > 0);
			return (size_t)(*mPtr & 15);
		}

		Item* _GetItems() const MOMO_NOEXCEPT
		{
			assert(_GetMemPoolIndex() > 0);
			return _GetItems(mPtr);
		}

		static Item* _GetItems(unsigned char* ptr) MOMO_NOEXCEPT
		{
			return (Item*)(ptr + ItemTraits::alignment);
		}

		Bounds _GetBounds() const MOMO_NOEXCEPT
		{
			if (mPtr == nullptr)
				return Bounds(nullptr, nullptr);
			else
				return Bounds(_GetItems(), _GetCount());
		}

	private:
		MOMO_DISABLE_COPY_CONSTRUCTOR(BucketLimP);
		MOMO_DISABLE_COPY_OPERATOR(BucketLimP);

	private:
		unsigned char* mPtr;
	};
}

template<size_t tMaxCount = 7,
	size_t tMemPoolBlockCount = MemPoolConst::defaultBlockCount>
struct HashBucketLimP : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;
	static const size_t memPoolBlockCount = tMemPoolBlockCount;

	template<typename ItemTraits, typename MemManager>
	struct Bucketer
	{
		typedef internal::BucketLimP<ItemTraits, MemManager,
			maxCount, memPoolBlockCount> Bucket;
	};
};

} // namespace momo

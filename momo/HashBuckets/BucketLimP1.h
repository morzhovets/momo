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
	template<typename TItemTraits, typename TMemManager,
		size_t tMaxCount, size_t tAlignment, size_t tMemPoolBlockCount>
	class BucketLimP1
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemManager MemManager;
		typedef typename ItemTraits::Item Item;

		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount < 16);

		static const size_t alignment = tAlignment;
		static const size_t memPoolBlockCount = tMemPoolBlockCount;

		typedef BucketBounds<Item> Bounds;
		typedef typename Bounds::ConstBounds ConstBounds;

	private:
		typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

		typedef momo::MemPool<MemPoolParamsVarSize<ItemTraits::alignment, memPoolBlockCount>,
			MemManagerPtr> MemPool;

		typedef BucketMemory<MemPool, Item*> Memory;

	public:
		class Params
		{
		public:
			static const bool skipFirstMemPool =
				(maxCount > 1 && memPoolBlockCount > 1 && ItemTraits::alignment == sizeof(Item));

		private:
			typedef momo::Array<MemPool, MemManagerDummy, ArrayItemTraits<MemPool>,
				ArraySettings<maxCount>> MemPools;

			static const size_t minMemPoolIndex = (skipFirstMemPool ? 2 : 1);

		public:
			Params(MemManager& memManager)
			{
				for (size_t i = minMemPoolIndex; i <= maxCount; ++i)
				{
					size_t blockSize = i * sizeof(Item);
					mMemPools.AddBackNogrow(MemPool(typename MemPool::Params(blockSize),
						MemManagerPtr(memManager)));
				}
			}

			MemPool& GetMemPool(size_t memPoolIndex) MOMO_NOEXCEPT
			{
				assert(memPoolIndex >= minMemPoolIndex);
				return mMemPools[memPoolIndex - minMemPoolIndex];
			}

		private:
			MOMO_DISABLE_COPY_CONSTRUCTOR(Params);
			MOMO_DISABLE_COPY_OPERATOR(Params);

		private:
			MemPools mMemPools;
		};

	public:
		BucketLimP1() MOMO_NOEXCEPT
		{
			_Set(nullptr, (unsigned char)0);
		}

		~BucketLimP1() MOMO_NOEXCEPT
		{
			assert(_GetItems() == nullptr);
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
			return _GetMemPoolIndex() == _GetMemPoolIndex(maxCount);
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			Item* items = _GetItems();
			if (items != nullptr)
			{
				ItemTraits::Destroy(items, _GetCount());
				params.GetMemPool(_GetMemPoolIndex()).Deallocate(items);
			}
			_Set(nullptr, (unsigned char)0);
		}

		template<typename ItemCreator>
		void AddBackCrt(Params& params, const ItemCreator& itemCreator)
		{
			Item* items = _GetItems();
			if (items == nullptr)
			{
				size_t newCount = 1;
				size_t newMemPoolIndex = _GetMemPoolIndex(newCount);
				Memory memory(params.GetMemPool(newMemPoolIndex));
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
					Memory memory(params.GetMemPool(newMemPoolIndex));
					ItemTraits::RelocateAddBack(items,
						memory.GetPointer(), count, itemCreator);
					params.GetMemPool(memPoolIndex).Deallocate(items);
					_Set(memory.Extract(), _MakeState(newMemPoolIndex, newCount));
				}
				else
				{
					itemCreator(items + count);
					++mState;
				}
			}
		}

		void RemoveBack(Params& params) MOMO_NOEXCEPT
		{
			size_t count = _GetCount();
			assert(count > 0);
			Item* items = _GetItems();
			ItemTraits::Destroy(items + count - 1, 1);
			if (count == 1 && !WasFull())
			{
				params.GetMemPool(_GetMemPoolIndex()).Deallocate(items);
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
			*&mItemPtrBuffer = items;
			mState = state;
		}

		static unsigned char _MakeState(size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			return (unsigned char)((memPoolIndex << 4) | count);
		}

		static size_t _GetMemPoolIndex(size_t count) MOMO_NOEXCEPT
		{
			assert(0 < count && count <= maxCount);
			if (Params::skipFirstMemPool && count == 1)
				return 2;
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

		Item* _GetItems() const MOMO_NOEXCEPT
		{
			return *&mItemPtrBuffer;
		}

	private:
		MOMO_DISABLE_COPY_CONSTRUCTOR(BucketLimP1);
		MOMO_DISABLE_COPY_OPERATOR(BucketLimP1);

	private:
		ObjectBuffer<Item*, alignment> mItemPtrBuffer;
		unsigned char mState;
	};
}

template<size_t tMaxCount = 7,
	size_t tAlignment = MOMO_ALIGNMENT_OF(void*),
	size_t tMemPoolBlockCount = MemPoolConst::defaultBlockCount>
struct HashBucketLimP1 : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;
	static const size_t alignment = tAlignment;
	static const size_t memPoolBlockCount = tMemPoolBlockCount;

	template<typename ItemTraits, typename MemManager>
	struct Bucketer
	{
		typedef internal::BucketLimP1<ItemTraits, MemManager,
			maxCount, alignment, memPoolBlockCount> Bucket;
	};
};

} // namespace momo

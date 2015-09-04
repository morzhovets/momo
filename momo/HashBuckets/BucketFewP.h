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
		uintptr_t tRemovedPtr, size_t tMemAlignment, size_t tMemPoolBlockCount>
	class BucketFewP
	{
	public:
		typedef TItemTraits ItemTraits;
		typedef TMemManager MemManager;
		typedef typename ItemTraits::Item Item;

		static const uintptr_t removedPtr = tRemovedPtr;
		static const size_t memPoolBlockCount = tMemPoolBlockCount;

		static const size_t memAlignment = tMemAlignment;
		MOMO_STATIC_ASSERT(memAlignment > 0 && ((memAlignment - 1) & memAlignment) == 0);

		static const uintptr_t nullPtr = (uintptr_t)nullptr;
		//MOMO_STATIC_ASSERT(removedPtr != nullPtr);	// llvm bug
		MOMO_STATIC_ASSERT(nullptr == (void*)0 && removedPtr != 0);

		static const size_t maxCount = (memAlignment >= 8) ? 3 : (memAlignment >= 4) ? 2 : 1;
		static const uintptr_t maskState = (memAlignment >= 8) ? 7 : (memAlignment >= 4) ? 3 : 0;

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
			typedef momo::Array<MemPool, MemManagerDummy, ArrayItemTraits<MemPool>,
				ArraySettings<maxCount>> MemPools;

		public:
			Params(MemManager& memManager)
			{
				for (size_t i = 1; i <= maxCount; ++i)
				{
					size_t blockSize = i * sizeof(Item);
					if (memPoolBlockCount > 1)
						blockSize = ((blockSize - 1) / memAlignment + 1) * memAlignment;
					else
						blockSize = std::minmax(blockSize, memAlignment).second;
					mMemPools.AddBackNogrow(MemPool(blockSize, MemManagerPtr(memManager)));
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
		BucketFewP() MOMO_NOEXCEPT
			: mPtr(nullPtr)
		{
		}

		~BucketFewP() MOMO_NOEXCEPT
		{
			assert(mPtr == nullPtr);
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
			if (mPtr == nullPtr)
				return false;
			if (mPtr == removedPtr)
				return true;
			return _GetMemPoolIndex() == maxCount;
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			if (mPtr != nullPtr && mPtr != removedPtr)
			{
				Item* items = _GetItems();
				ItemTraits::Destroy(items, _GetCount());
				params[_GetMemPoolIndex()].FreeMemory(items);
			}
			mPtr = nullPtr;
		}

		template<typename ItemCreator>
		void AddBackEmpl(Params& params, const ItemCreator& itemCreator)
		{
			if (mPtr == nullPtr || mPtr == removedPtr)
			{
				size_t newCount = 1;
				size_t newMemPoolIndex = (mPtr == nullPtr) ? 1 : maxCount;
				Memory memory(params[newMemPoolIndex]);
				_CheckMemory(memory);
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
					size_t newMemPoolIndex = newCount;
					Memory memory(params[newMemPoolIndex]);
					_CheckMemory(memory);
					Item* items = _GetItems();
					ItemTraits::RelocateAddBack(items, memory.GetPointer(),
						count, itemCreator);
					params[memPoolIndex].FreeMemory(items);
					_Set(memory.Extract(), newMemPoolIndex, newCount);
				}
				else
				{
					itemCreator(_GetItems() + count);
					++mPtr;
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
				params[memPoolIndex].FreeMemory(items);
				mPtr = (memPoolIndex == maxCount) ? removedPtr : nullPtr;
			}
			else
			{
				--mPtr;
			}
		}

	private:
		static void _CheckMemory(const Memory& memory)
		{
			if (((uintptr_t)memory.GetPointer() & maskState) != 0)
				throw std::bad_alloc();
		}

		void _Set(Item* items, size_t memPoolIndex, size_t count) MOMO_NOEXCEPT
		{
			mPtr = (uintptr_t)items;
			mPtr |= (uintptr_t)((maxCount - memPoolIndex) * maxCount + count - 1);
		}

		size_t _GetMemPoolIndex() const MOMO_NOEXCEPT
		{
			assert(mPtr != nullPtr && mPtr != removedPtr);
			return maxCount - (size_t)(mPtr & maskState) / maxCount;
		}

		size_t _GetCount() const MOMO_NOEXCEPT
		{
			if (mPtr == nullPtr || mPtr == removedPtr)
				return 0;
			return (size_t)(mPtr & maskState) % maxCount + 1;
		}

		Item* _GetItems() const MOMO_NOEXCEPT
		{
			if (mPtr == nullPtr || mPtr == removedPtr)
				return nullptr;
			return (Item*)(mPtr & ~maskState);
		}

	private:
		MOMO_DISABLE_COPY_CONSTRUCTOR(BucketFewP);
		MOMO_DISABLE_COPY_OPERATOR(BucketFewP);

	private:
		uintptr_t mPtr;
	};
}

template<uintptr_t tRemovedPtr,
	size_t tMemAlignment = 8,
	size_t tMemPoolBlockCount = MemPoolConst::defaultBlockCount>
struct HashBucketFewP
	: public internal::HashBucketBase<(tMemAlignment >= 8) ? 3 : (tMemAlignment >= 4) ? 2 : 1>
{
	static const uintptr_t removedPtr = tRemovedPtr;
	static const size_t memAlignment = tMemAlignment;
	static const size_t memPoolBlockCount = tMemPoolBlockCount;

	template<typename ItemTraits, typename MemManager>
	struct Bucketer
	{
		typedef internal::BucketFewP<ItemTraits, MemManager,
			removedPtr, memAlignment, memPoolBlockCount> Bucket;
	};
};

} // namespace momo

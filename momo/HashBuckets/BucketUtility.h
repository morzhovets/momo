/**********************************************************\

  momo/HashBuckets/BucketUtility.h

\**********************************************************/

#pragma once

#include "../Utility.h"

namespace momo
{

namespace internal
{
	template<typename TItem>
	class BucketBounds
	{
	public:
		typedef TItem Item;

		typedef Item* Iterator;

		typedef BucketBounds<const Item> ConstBounds;

	public:
		BucketBounds() MOMO_NOEXCEPT
			: mBegin(nullptr),
			mEnd(nullptr)
		{
		}

		BucketBounds(Item* begin, Item* end) MOMO_NOEXCEPT
			: mBegin(begin),
			mEnd(end)
		{
		}

		BucketBounds(Item* begin, size_t count) MOMO_NOEXCEPT
			: mBegin(begin),
			mEnd(begin + count)
		{
		}

		operator ConstBounds() const MOMO_NOEXCEPT
		{
			return ConstBounds(mBegin, mEnd);
		}

		Item* GetBegin() const MOMO_NOEXCEPT
		{
			return mBegin;
		}

		Item* GetEnd() const MOMO_NOEXCEPT
		{
			return mEnd;
		}

		MOMO_FRIENDS_BEGIN_END(const BucketBounds&, Item*)

		size_t GetCount() const MOMO_NOEXCEPT
		{
			return mEnd - mBegin;
		}

		Item& operator[](size_t index) const MOMO_NOEXCEPT
		{
			assert(index < GetCount());
			return mBegin[index];
		}

	private:
		Item* mBegin;
		Item* mEnd;
	};

	template<typename TMemPool, typename TPointer,
		TPointer nullPtr = TPointer(nullptr)>
	class BucketMemory
	{
	public:
		typedef TMemPool MemPool;
		typedef TPointer Pointer;

	public:
		explicit BucketMemory(MemPool& memPool)
			: mMemPool(memPool),
			mPtr((Pointer)memPool.GetMemory())
		{
			assert(mPtr != nullPtr);
		}

		~BucketMemory() MOMO_NOEXCEPT
		{
			if (mPtr != nullPtr)
				mMemPool.FreeMemory(mPtr);
		}

		Pointer GetPointer() const MOMO_NOEXCEPT
		{
			return mPtr;
		}

		Pointer Extract() MOMO_NOEXCEPT
		{
			Pointer ptr = mPtr;
			mPtr = nullPtr;
			return ptr;
		}

	private:
		MOMO_DISABLE_COPY_CONSTRUCTOR(BucketMemory);
		MOMO_DISABLE_COPY_OPERATOR(BucketMemory);

	private:
		MemPool& mMemPool;
		Pointer mPtr;
	};

	class MemManagerDummy
	{
	public:
		static const bool canReallocate = false;
		static const bool canReallocateInplace = false;
	
	public:
		//void* Allocate(size_t size);

		void Deallocate(void* /*ptr*/, size_t /*size*/) MOMO_NOEXCEPT
		{
			assert(false);
		}
	
	private:
		MOMO_DISABLE_COPY_OPERATOR(MemManagerDummy);
	};
}

} // namespace momo

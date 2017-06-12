/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/BucketUtility.h

  namespace momo:
    enum class HashBucketOneState

\**********************************************************/

#pragma once

#include "../IteratorUtility.h"

namespace momo
{

namespace internal
{
	template<typename TIterator>
	class BucketBounds
	{
	public:
		typedef TIterator Iterator;

		typedef BucketBounds<typename ConstIteratorSelector<Iterator>::ConstIterator> ConstBounds;

		typedef typename std::iterator_traits<Iterator>::reference Reference;

	public:
		BucketBounds() MOMO_NOEXCEPT
			: mBegin(nullptr),
			mCount(0)
		{
		}

		BucketBounds(Iterator begin, size_t count) MOMO_NOEXCEPT
			: mBegin(begin),
			mCount(count)
		{
		}

		operator ConstBounds() const MOMO_NOEXCEPT
		{
			return ConstBounds(mBegin, mCount);
		}

		Iterator GetBegin() const MOMO_NOEXCEPT
		{
			return mBegin;
		}

		Iterator GetEnd() const MOMO_NOEXCEPT
		{
			return mBegin + mCount;
		}

		MOMO_FRIENDS_BEGIN_END(const BucketBounds&, Iterator)

		size_t GetCount() const MOMO_NOEXCEPT
		{
			return mCount;
		}

		Reference operator[](size_t index) const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(index < mCount);
			return mBegin[index];
		}

	private:
		Iterator mBegin;
		size_t mCount;
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
			mPtr(static_cast<Pointer>(memPool.Allocate()))
		{
			MOMO_ASSERT(mPtr != nullPtr);
		}

		BucketMemory(const BucketMemory&) = delete;

		~BucketMemory() MOMO_NOEXCEPT
		{
			if (mPtr != nullPtr)
				mMemPool.Deallocate(mPtr);
		}

		BucketMemory& operator=(const BucketMemory&) = delete;

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
		MemPool& mMemPool;
		Pointer mPtr;
	};

	template<typename TMemManager>
	class BucketParamsOpen
	{
	public:
		typedef TMemManager MemManager;

	public:
		explicit BucketParamsOpen(MemManager& memManager) MOMO_NOEXCEPT
			: mMemManager(memManager)
		{
		}

		BucketParamsOpen(const BucketParamsOpen&) = delete;

		~BucketParamsOpen() MOMO_NOEXCEPT
		{
		}

		BucketParamsOpen& operator=(const BucketParamsOpen&) = delete;

		MemManager& GetMemManager() MOMO_NOEXCEPT
		{
			return mMemManager;
		}

	private:
		MemManager& mMemManager;
	};

	template<size_t tMaxCount>
	struct HashBucketBase
	{
		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(maxCount > 0);

		static const size_t logStartBucketCount = 4;

		static size_t CalcCapacity(size_t bucketCount) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(bucketCount > 0);
			if (maxCount == 1)
				return (bucketCount / 8) * 5;
			else if (maxCount == 2)
				return bucketCount + bucketCount / 2;
			else
				return bucketCount * 2;
		}

		static size_t GetBucketCountShift(size_t bucketCount) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(bucketCount > 0);
			if (maxCount == 1)
				return 1;
			else if (maxCount == 2)
				return (bucketCount < (1 << 16)) ? 2 : 1;
			else
				return (bucketCount < (1 << 20)) ? 2 : 1;
		}

		static size_t GetBucketIndex(size_t hashCode, size_t bucketCount,
			size_t probe) MOMO_NOEXCEPT
		{
			return (hashCode + probe) & (bucketCount - 1);	// linear probing
		}

		static void CheckMaxLoadFactor(float maxLoadFactor)
		{
			if (maxLoadFactor <= 0 || maxLoadFactor > (float)maxCount)
				throw std::out_of_range("invalid hash load factor");
		}
	};
}

enum class HashBucketOneState : uint8_t
{
	empty = 0,
	full = 1,
	removed = 2,
};

} // namespace momo

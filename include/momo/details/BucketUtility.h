/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/details/BucketUtility.h

\**********************************************************/

#pragma once

#include "../IteratorUtility.h"

namespace momo
{

namespace internal
{
	template<typename TMemPool, typename TPointer,
		TPointer nullPtr = TPointer(nullptr)>
	class BucketMemory
	{
	public:
		typedef TMemPool MemPool;
		typedef TPointer Pointer;

	public:
		explicit BucketMemory(MemPool& memPool)
			: mMemPool(memPool)
		{
			auto ptr = memPool.Allocate();
			MOMO_ASSERT(ptr != nullPtr);
			if constexpr (std::is_pointer_v<Pointer>)
				mPtr = PtrCaster::FromBytePtr<std::remove_pointer_t<Pointer>>(ptr);
			else
				mPtr = ptr;
		}

		BucketMemory(const BucketMemory&) = delete;

		~BucketMemory() noexcept
		{
			if (mPtr != nullPtr)
				mMemPool.Deallocate(mPtr);
		}

		BucketMemory& operator=(const BucketMemory&) = delete;

		Pointer Get() const noexcept
		{
			return mPtr;
		}

		Pointer Extract() noexcept
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
		explicit BucketParamsOpen(MemManager& memManager) noexcept
			: mMemManager(memManager)
		{
		}

		BucketParamsOpen(const BucketParamsOpen&) = delete;

		~BucketParamsOpen() noexcept = default;

		BucketParamsOpen& operator=(const BucketParamsOpen&) = delete;

		void Clear() noexcept
		{
		}

		MemManager& GetMemManager() noexcept
		{
			return mMemManager;
		}

	private:
		MemManager& mMemManager;
	};

	class BucketBase
	{
	public:
		template<bool first, conceptMutableThis Bucket,
			conceptObjectPredicate<typename Bucket::Item> ItemPredicate,
			typename Iterator = typename Bucket::Iterator>
		MOMO_FORCEINLINE Iterator Find(this Bucket& bucket, typename Bucket::Params& params,
			FastCopyableFunctor<ItemPredicate> itemPred, size_t /*hashCode*/)
		{
			typename Bucket::Bounds bounds = bucket.GetBounds(params);
			for (Iterator iter = bounds.GetBegin(), end = bounds.GetEnd(); iter != end; ++iter)
			{
				if (itemPred(std::as_const(*iter)))
					return iter;
			}
			return Iterator();
		}

		size_t GetMaxProbe(size_t logBucketCount) const noexcept
		{
			return (size_t{1} << logBucketCount) - 1;
		}

		void UpdateMaxProbe(size_t /*probe*/) noexcept
		{
		}

		template<typename Bucket, conceptConstFunctor<size_t> HashCodeFullGetter>
		size_t GetHashCodePart(this const Bucket&,
			FastCopyableFunctor<HashCodeFullGetter> hashCodeFullGetter,
			typename Bucket::Iterator /*iter*/, size_t /*bucketIndex*/,
			size_t /*logBucketCount*/, size_t /*newLogBucketCount*/)
		{
			return hashCodeFullGetter();
		}

		static size_t GetStartBucketIndex(size_t hashCode, size_t bucketCount) noexcept
		{
			return hashCode & (bucketCount - 1);
		}

		static size_t GetNextBucketIndex(size_t bucketIndex, size_t /*hashCode*/,
			size_t bucketCount, size_t /*probe*/) noexcept
		{
			return (bucketIndex + 1) & (bucketCount - 1);	// linear probing
		}
	};

	class HashBucketBase
	{
	public:
		static const size_t logStartBucketCount = 4;

	public:
		static size_t CalcCapacity(size_t bucketCount, size_t bucketMaxItemCount) noexcept
		{
			MOMO_ASSERT(bucketCount > 0 && bucketMaxItemCount > 0);
			if (bucketMaxItemCount == 1)
				return static_cast<size_t>(static_cast<double>(bucketCount) / 8.0 * 5.0);
			else if (bucketMaxItemCount == 2)
				return bucketCount + bucketCount / 2;
			else
				return bucketCount * 2;
		}

		static size_t GetBucketCountShift(size_t bucketCount,
			size_t bucketMaxItemCount) noexcept
		{
			MOMO_ASSERT(bucketCount > 0 && bucketMaxItemCount > 0);
			if (bucketMaxItemCount == 1)
				return 1;
			else if (bucketMaxItemCount == 2)
				return (bucketCount < (1 << 16)) ? 2 : 1;
			else
				return (bucketCount < (1 << 20)) ? 2 : 1;
		}
	};
}

} // namespace momo

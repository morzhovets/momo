/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketOpenN1.h

  namespace momo:
    struct HashBucketOpenN1

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../ObjectManager.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, size_t tMaxCount>
	class BucketOpenN1 : public BucketBase
	{
	protected:
		typedef TItemTraits ItemTraits;

	public:
		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount < 8);

		static const bool isNothrowAddableIfNothrowCreatable = true;

		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef std::reverse_iterator<Item*> Iterator;
		typedef ArrayBounds<Iterator> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

	private:
		static const uint8_t emptyShortHash = 248;
		static const uint8_t infProbe = 255;

	public:
		explicit BucketOpenN1() MOMO_NOEXCEPT
		{
			pvSetEmpty();
		}

		BucketOpenN1(const BucketOpenN1&) = delete;

		~BucketOpenN1() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(pvGetCount() == 0);
		}

		BucketOpenN1& operator=(const BucketOpenN1&) = delete;

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return Bounds(Iterator(&mItems[0] + maxCount), pvGetCount());
		}

		template<typename Predicate>
		Iterator Find(Params& /*params*/, const Predicate& pred, size_t hashCode)
		{
			uint8_t shortHash = pvGetShortHash(hashCode);
			for (size_t i = 0; i < maxCount; ++i)
			{
				if (mShortHashes[i] == shortHash && pred(*&mItems[i]))
					return Iterator(&mItems[i] + 1);
			}
			return Iterator(nullptr);
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return mState < emptyShortHash;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return true;
		}

		size_t GetMaxProbe(size_t logBucketCount) const MOMO_NOEXCEPT
		{
			if (mMaxProbe == infProbe)
				return ((size_t)1 << logBucketCount) - 1;
			return (size_t)(mMaxProbe & 7) << (mMaxProbe >> 3);
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			size_t count = pvGetCount();
			ItemTraits::Destroy(params.GetMemManager(), &mItems[0] + maxCount - count, count);
			pvSetEmpty();
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& /*params*/, ItemCreator&& itemCreator, size_t hashCode,
			size_t logBucketCount, size_t probe)
			MOMO_NOEXCEPT_IF(noexcept(std::forward<ItemCreator>(itemCreator)(std::declval<Item*>())))
		{
			size_t count = pvGetCount();
			MOMO_ASSERT(count < maxCount);
			Item* pitem = &mItems[maxCount - 1 - count];
			std::forward<ItemCreator>(itemCreator)(pitem);
			mShortHashes[maxCount - 1 - count] = pvGetShortHash(hashCode);
			if (probe > 0)
				pvSetMaxProbe(hashCode, logBucketCount, probe);
			if (count + 1 < maxCount)
				++mState;
			return Iterator(pitem + 1);
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, ItemReplacer&& itemReplacer)
		{
			size_t count = pvGetCount();
			size_t index = iter - Iterator(&mItems[0] + maxCount);
			MOMO_ASSERT(index < count);
			std::forward<ItemReplacer>(itemReplacer)(*&mItems[maxCount - count],
				*&mItems[maxCount - 1 - index]);
			mShortHashes[maxCount - 1 - index] = mShortHashes[maxCount - count];
			mShortHashes[maxCount - count] = emptyShortHash;
			if (count < maxCount)
				--mState;
			else
				mState = emptyShortHash + (uint8_t)maxCount - 1;
			return iter;
		}

	private:
		size_t pvGetCount() const MOMO_NOEXCEPT
		{
			if (IsFull())
				return maxCount;
			return (size_t)(mState - emptyShortHash);
		}

		static uint8_t pvGetShortHash(size_t hashCode) MOMO_NOEXCEPT
		{
			uint32_t hashCode24 = (uint32_t)(hashCode >> (sizeof(size_t) * 8 - 24));
			return (uint8_t)((hashCode24 * (uint32_t)emptyShortHash) >> 24);
		}

		void pvSetEmpty() MOMO_NOEXCEPT
		{
			mMaxProbe = (uint8_t)0;
			std::fill_n(mShortHashes, maxCount, (uint8_t)emptyShortHash);
		}

		void pvSetMaxProbe(size_t hashCode, size_t logBucketCount, size_t probe) MOMO_NOEXCEPT
		{
			size_t bucketCount = (size_t)1 << logBucketCount;
			size_t startBucketIndex = BucketBase::GetStartBucketIndex(hashCode, bucketCount);
			size_t thisBucketIndex = (startBucketIndex + probe) & (bucketCount - 1);
			BucketOpenN1* startBucket = this - (ptrdiff_t)thisBucketIndex
				+ (ptrdiff_t)startBucketIndex;
			if (probe <= startBucket->GetMaxProbe(logBucketCount))
				return;
			size_t maxProbe0 = probe - 1;
			size_t maxProbe1 = 0;
			while (maxProbe0 >= (size_t)7)
			{
				maxProbe0 >>= 1;
				++maxProbe1;
			}
			if (maxProbe1 <= (size_t)31)
				startBucket->mMaxProbe = (uint8_t)(maxProbe0 + 1) | (uint8_t)(maxProbe1 << 3);
			else
				startBucket->mMaxProbe = infProbe;
		}

	private:
		uint8_t mMaxProbe;
		union
		{
			uint8_t mState;
			uint8_t mShortHashes[maxCount];
		};
		ObjectBuffer<Item, ItemTraits::alignment> mItems[maxCount];
	};
}

template<size_t tMaxCount = 3>
struct HashBucketOpenN1 : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;

	static size_t CalcCapacity(size_t bucketCount) MOMO_NOEXCEPT
	{
		return (bucketCount * maxCount / 6) * 5;
	}

	static size_t GetBucketCountShift(size_t /*bucketCount*/) MOMO_NOEXCEPT
	{
		return 1;
	}

	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = internal::BucketOpenN1<ItemTraits, maxCount>;
};

} // namespace momo

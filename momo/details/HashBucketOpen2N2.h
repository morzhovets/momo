/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketOpen2N2.h

  namespace momo:
    struct HashBucketOpen2N2

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../ObjectManager.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, size_t tMaxCount, bool tUseHashCodePartGetter>
	class BucketOpen2N2
	{
	protected:
		typedef TItemTraits ItemTraits;

		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount < 4);

		static const bool useHashCodePartGetter = tUseHashCodePartGetter;

	public:
		typedef typename ItemTraits::Item Item;
		typedef typename ItemTraits::MemManager MemManager;

		typedef std::reverse_iterator<Item*> Iterator;
		typedef BucketBounds<Iterator> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

	private:
		static const uint8_t emptyHash = 128;
		static const uint8_t emptyHashProbe = 255;

		static const size_t logBucketCountStep = 8;
		static const size_t logBucketCountAddend = 6;
		static const size_t hashCodeShift = sizeof(size_t) * 8 - 7;

	public:
		BucketOpen2N2() MOMO_NOEXCEPT
		{
			pvSetEmpty();
		}

		BucketOpen2N2(const BucketOpen2N2&) = delete;

		~BucketOpen2N2() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(pvGetCount() == 0);
		}

		BucketOpen2N2& operator=(const BucketOpen2N2&) = delete;

		Bounds GetBounds(Params& /*params*/) MOMO_NOEXCEPT
		{
			return Bounds(Iterator(&mItems[0] + maxCount), pvGetCount());
		}

		template<typename Predicate>
		Iterator Find(Params& /*params*/, const Predicate& pred, size_t hashCode,
			size_t /*logBucketCount*/)
		{
			uint8_t hashByte = (uint8_t)(hashCode >> hashCodeShift);
			for (size_t i = 0; i < maxCount; ++i)
			{
				if (mHashes[i] == hashByte && pred(*&mItems[i]))
					return Iterator(&mItems[i] + 1);
			}
			return Iterator(nullptr);
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return mHashes[0] < emptyHash;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return true;
		}

		size_t GetMaxProbe(size_t /*logBucketCount*/) const MOMO_NOEXCEPT
		{
			return (size_t)mMaxProbe[0] << mMaxProbe[1];
		}

		void Clear(Params& params) MOMO_NOEXCEPT
		{
			size_t count = pvGetCount();
			ItemTraits::Destroy(params.GetMemManager(), &mItems[0] + maxCount - count, count);
			pvSetEmpty();
		}

		template<typename ItemCreator>
		Iterator AddCrt(Params& /*params*/, const ItemCreator& itemCreator, size_t hashCode,
			size_t logBucketCount, size_t probe)
		{
			size_t count = pvGetCount();
			MOMO_ASSERT(count < maxCount);
			Item* pitem = &mItems[maxCount - 1 - count];
			itemCreator(pitem);
			mHashes[maxCount - 1 - count] = (uint8_t)(hashCode >> hashCodeShift);
			if (useHashCodePartGetter)
			{
				uint8_t& hashProbe = mHashProbes[maxCount - 1 - count];
				size_t probeShift = pvGetProbeShift(logBucketCount);
				if (probe < ((size_t)1 << probeShift))
					hashProbe = (uint8_t)(((hashCode >> logBucketCount) << probeShift) | probe);
				else
					hashProbe = emptyHashProbe;
			}
			if (probe > 0)
				pvSetMaxProbe(hashCode, logBucketCount, probe);
			return Iterator(pitem + 1);
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, const ItemReplacer& itemReplacer)
		{
			size_t count = pvGetCount();
			size_t index = std::addressof(*iter) - &mItems[0];
			MOMO_ASSERT(index >= maxCount - count);
			itemReplacer(*&mItems[maxCount - count], *&mItems[index]);
			mHashes[index] = mHashes[maxCount - count];
			mHashes[maxCount - count] = emptyHash;
			if (useHashCodePartGetter)
				mHashProbes[index] = mHashProbes[maxCount - count];
			return iter;
		}

		template<typename HashCodeFullGetter>
		size_t GetHashCodePart(const HashCodeFullGetter& hashCodeFullGetter, Iterator iter,
			size_t bucketIndex, size_t logBucketCount, size_t newLogBucketCount)
		{
			if (!useHashCodePartGetter)
				return hashCodeFullGetter();
			size_t index = std::addressof(*iter) - &mItems[0];
			uint8_t hashProbe = mHashProbes[index];
			bool useFullGetter = (hashProbe == emptyHashProbe ||
				(logBucketCount + logBucketCountAddend) / logBucketCountStep
				!= (newLogBucketCount + logBucketCountAddend) / logBucketCountStep);
			if (useFullGetter)
				return hashCodeFullGetter();
			size_t probeShift = pvGetProbeShift(logBucketCount);
			MOMO_ASSERT(probeShift > 0);
			size_t probe = (size_t)hashProbe & (((size_t)1 << probeShift) - 1);
			size_t bucketCount = (size_t)1 << logBucketCount;
			return ((bucketIndex + bucketCount - probe) & (bucketCount - 1))
				| (((size_t)hashProbe >> probeShift) << logBucketCount)
				| ((size_t)mHashes[index] << hashCodeShift);
		}

	private:
		size_t pvGetCount() const MOMO_NOEXCEPT
		{
			switch (maxCount)
			{
			case 1:
				return (mHashes[0] < emptyHash) ? 1 : 0;
			case 2:
				return (mHashes[1] < emptyHash) ? ((mHashes[0] < emptyHash) ? 2 : 1) : 0;
			default:
				return (mHashes[1] < emptyHash) ? ((mHashes[0] < emptyHash) ? 3 : 2)
					: ((mHashes[2] < emptyHash) ? 1 : 0);
			}
		}

		void pvSetEmpty() MOMO_NOEXCEPT
		{
			std::fill_n(mHashes, maxCount, (uint8_t)emptyHash);
			mMaxProbe[0] = mMaxProbe[1] = (uint8_t)0;
		}

		static size_t pvGetProbeShift(size_t logBucketCount) MOMO_NOEXCEPT
		{
			return (logBucketCount + logBucketCountAddend + 1) % logBucketCountStep;
		}

		void pvSetMaxProbe(size_t hashCode, size_t logBucketCount, size_t probe) MOMO_NOEXCEPT
		{
			size_t bucketCount = (size_t)1 << logBucketCount;
			size_t trueBucketIndex = hashCode & (bucketCount - 1);
			size_t thisBucketIndex = (hashCode + probe) & (bucketCount - 1);
			BucketOpen2N2* trueBucket = this - (ptrdiff_t)thisBucketIndex
				+ (ptrdiff_t)trueBucketIndex;
			if (probe <= trueBucket->GetMaxProbe(logBucketCount))
				return;
			if (probe <= (size_t)255)
			{
				trueBucket->mMaxProbe[0] = (uint8_t)probe;
				return;
			}
			size_t maxProbe0 = probe - 1;
			size_t maxProbe1 = 0;
			while (maxProbe0 >= (size_t)255)
			{
				maxProbe0 >>= 1;
				++maxProbe1;
			}
			trueBucket->mMaxProbe[0] = (uint8_t)maxProbe0 + 1;
			trueBucket->mMaxProbe[1] = (uint8_t)maxProbe1;
		}

	private:
		uint8_t mHashes[maxCount];
		uint8_t mHashProbes[maxCount];
		uint8_t mMaxProbe[2];
		ObjectBuffer<Item, ItemTraits::alignment> mItems[maxCount];
	};
}

template<size_t tMaxCount = 3,
	bool tUseHashCodePartGetter = true>
struct HashBucketOpen2N2 : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;
	static const bool useHashCodePartGetter = tUseHashCodePartGetter;

	static size_t CalcCapacity(size_t bucketCount) MOMO_NOEXCEPT
	{
		return (bucketCount * maxCount / 6) * 5;
	}

	static size_t GetBucketCountShift(size_t /*bucketCount*/) MOMO_NOEXCEPT
	{
		return 1;
	}

	template<typename ItemTraits>
	using Bucket = internal::BucketOpen2N2<ItemTraits, maxCount, useHashCodePartGetter>;
};

} // namespace momo

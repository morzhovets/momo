/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketOpen2N.h

  namespace momo:
    struct HashBucketOpen2N

\**********************************************************/

#pragma once

#include "BucketUtility.h"
#include "../ObjectManager.h"

namespace momo
{

namespace internal
{
	template<typename TItemTraits, size_t tMaxCount, bool tUseHashCodePartGetter>
	class BucketOpen2N
	{
	protected:
		typedef TItemTraits ItemTraits;

		static const size_t maxCount = tMaxCount;
		MOMO_STATIC_ASSERT(0 < maxCount && maxCount < 64);

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
		static const uint8_t maskCount = 63;

		static const size_t logBucketCountStep = 8;
		static const size_t logBucketCountAddend = 6;
		static const size_t hashCodeShift = sizeof(size_t) * 8 - 7;

	public:
		BucketOpen2N() MOMO_NOEXCEPT
		{
			pvSetEmpty();
		}

		BucketOpen2N(const BucketOpen2N&) = delete;

		~BucketOpen2N() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(pvGetCount() == 0);
		}

		BucketOpen2N& operator=(const BucketOpen2N&) = delete;

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
				if (mHashProbes[i][0] == hashByte && pred(*&mItems[i]))
					return Iterator(&mItems[i] + 1);
			}
			return Iterator(nullptr);
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return mState < emptyHash;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			if (mState < emptyHash)
				return true;
			return (mState & (maskCount + 1)) != (uint8_t)0;
		}

		size_t GetMaxProbe(size_t logBucketCount) const MOMO_NOEXCEPT
		{
			return ((size_t)1 << logBucketCount) - 1;
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
			uint8_t* hashProbe = mHashProbes[maxCount - 1 - count];
			hashProbe[0] = (uint8_t)(hashCode >> hashCodeShift);
			if (useHashCodePartGetter)
			{
				size_t probeShift = pvGetProbeShift(logBucketCount);
				if (probe < ((size_t)1 << probeShift))
					hashProbe[1] = (uint8_t)(((hashCode >> logBucketCount) << probeShift) | probe);
				else
					hashProbe[1] = emptyHashProbe;
			}
			if (count + 1 < maxCount)
				++mState;
			return Iterator(pitem + 1);
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, const ItemReplacer& itemReplacer)
		{
			size_t count = pvGetCount();
			size_t index = std::addressof(*iter) - &mItems[0];
			MOMO_ASSERT(index >= maxCount - count);
			itemReplacer(*&mItems[maxCount - count], *&mItems[index]);
			mHashProbes[index][0] = mHashProbes[maxCount - count][0];
			if (useHashCodePartGetter)
				mHashProbes[index][1] = mHashProbes[maxCount - count][1];
			mHashProbes[maxCount - count][0] = emptyHash;
			if (count < maxCount)
				--mState;
			else
				mState = emptyHash + maskCount + (uint8_t)maxCount;
			return iter;
		}

		template<typename HashCodeFullGetter>
		size_t GetHashCodePart(const HashCodeFullGetter& hashCodeFullGetter, Iterator iter,
			size_t bucketIndex, size_t logBucketCount, size_t newLogBucketCount)
		{
			if (!useHashCodePartGetter)
				return hashCodeFullGetter();
			size_t index = std::addressof(*iter) - &mItems[0];
			uint8_t* hashProbe = mHashProbes[index];
			bool useFullGetter = (hashProbe[1] == emptyHashProbe ||
				(logBucketCount + logBucketCountAddend) / logBucketCountStep
				!= (newLogBucketCount + logBucketCountAddend) / logBucketCountStep);
			if (useFullGetter)
				return hashCodeFullGetter();
			size_t probeShift = pvGetProbeShift(logBucketCount);
			MOMO_ASSERT(probeShift > 0);
			size_t probe = (size_t)hashProbe[1] & (((size_t)1 << probeShift) - 1);
			size_t bucketCount = (size_t)1 << logBucketCount;
			return ((bucketIndex + bucketCount - probe) & (bucketCount - 1))
				| (((size_t)hashProbe[1] >> probeShift) << logBucketCount)
				| ((size_t)hashProbe[0] << hashCodeShift);
		}

	private:
		size_t pvGetCount() const MOMO_NOEXCEPT
		{
			if (mState < emptyHash)
				return maxCount;
			return (size_t)(mState & maskCount);
		}

		void pvSetEmpty() MOMO_NOEXCEPT
		{
			for (size_t i = 0; i < maxCount; ++i)
				mHashProbes[i][0] = emptyHash;
		}

		static size_t pvGetProbeShift(size_t logBucketCount) MOMO_NOEXCEPT
		{
			return (logBucketCount + logBucketCountAddend + 1) % logBucketCountStep;
		}

	private:
		union
		{
			uint8_t mState;
			uint8_t mHashProbes[maxCount][2];
		};
		ObjectBuffer<Item, ItemTraits::alignment> mItems[maxCount];
	};
}

template<size_t tMaxCount = 4,
	bool tUseHashCodePartGetter = true>
struct HashBucketOpen2N : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;
	static const bool useHashCodePartGetter = tUseHashCodePartGetter;

	static size_t CalcCapacity(size_t bucketCount) MOMO_NOEXCEPT
	{
		return (bucketCount * maxCount / 8) * 5;
	}

	static size_t GetBucketCountShift(size_t /*bucketCount*/) MOMO_NOEXCEPT
	{
		return 1;
	}

	template<typename ItemTraits>
	using Bucket = internal::BucketOpen2N<ItemTraits, maxCount, useHashCodePartGetter>;
};

} // namespace momo

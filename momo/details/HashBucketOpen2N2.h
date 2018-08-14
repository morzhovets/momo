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
	class BucketOpen2N2 : public BucketBase<tMaxCount>
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
		typedef ArrayBounds<Iterator> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

		static const bool isNothrowAddableIfNothrowCreatable = true;

	private:
		template<size_t count, bool useHashCodePartGetter>
		struct HashData;

		template<size_t count>
		struct HashData<count, false>
		{
			union
			{
				uint16_t shortHashes[count];
				uint8_t hashProbes[count];
			};
		};

		template<size_t count>
		struct HashData<count, true>
		{
			uint8_t shortHashes[count];
			uint8_t hashProbes[count];
		};

		typedef typename UIntSelector<useHashCodePartGetter ? 1 : 2>::UInt ShortHash;

		static const size_t hashCodeShift = sizeof(size_t) * 8 - sizeof(ShortHash) * 8 + 1;
		static const ShortHash emptyShortHash = (ShortHash)1 << (sizeof(ShortHash) * 8 - 1);
		static const uint8_t emptyHashProbe = 255;

		static const size_t logBucketCountStep = 8;
		static const size_t logBucketCountAddend = 6;

	public:
		explicit BucketOpen2N2() MOMO_NOEXCEPT
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
		Iterator Find(Params& /*params*/, const Predicate& pred, size_t hashCode)
		{
			ShortHash shortHash = pvGetShortHash(hashCode);
			for (size_t i = 0; i < maxCount; ++i)
			{
				if (mHashData.shortHashes[i] == shortHash && pred(*&mItems[i]))
					return Iterator(&mItems[i] + 1);
			}
			return Iterator(nullptr);
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return mHashData.shortHashes[0] < emptyShortHash;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			return true;
		}

		size_t GetMaxProbe(size_t /*logBucketCount*/) const MOMO_NOEXCEPT
		{
			return (size_t)mState[0] << (mState[1] >> 2);
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
			mHashData.shortHashes[maxCount - 1 - count] = pvGetShortHash(hashCode);
			if (useHashCodePartGetter)
			{
				uint8_t& hashProbe = mHashData.hashProbes[maxCount - 1 - count];
				size_t probeShift = pvGetProbeShift(logBucketCount);
				if (probe < ((size_t)1 << probeShift))
					hashProbe = (uint8_t)(((hashCode >> logBucketCount) << probeShift) | probe);
				else
					hashProbe = emptyHashProbe;
			}
			if (probe > 0)
				pvSetMaxProbe(hashCode, logBucketCount, probe);
			++mState[1];
			return Iterator(pitem + 1);
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, ItemReplacer&& itemReplacer)
		{
			size_t count = pvGetCount();
			size_t index = std::addressof(*iter) - &mItems[0];
			MOMO_ASSERT(index >= maxCount - count);
			std::forward<ItemReplacer>(itemReplacer)(*&mItems[maxCount - count], *&mItems[index]);
			mHashData.shortHashes[index] = mHashData.shortHashes[maxCount - count];
			mHashData.shortHashes[maxCount - count] = emptyShortHash;
			if (useHashCodePartGetter)
				mHashData.hashProbes[index] = mHashData.hashProbes[maxCount - count];
			--mState[1];
			return iter;
		}

		template<typename HashCodeFullGetter>
		size_t GetHashCodePart(const HashCodeFullGetter& hashCodeFullGetter, Iterator iter,
			size_t bucketIndex, size_t logBucketCount, size_t newLogBucketCount)
		{
			if (!useHashCodePartGetter)
				return hashCodeFullGetter();
			size_t index = std::addressof(*iter) - &mItems[0];
			uint8_t hashProbe = mHashData.hashProbes[index];
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
				| ((size_t)mHashData.shortHashes[index] << hashCodeShift);
		}

		static size_t GetNextBucketIndex(size_t bucketIndex, size_t /*hashCode*/,
			size_t bucketCount, size_t /*probe*/) MOMO_NOEXCEPT
		{
			return (bucketIndex + 1) & (bucketCount - 1);
		}

	private:
		size_t pvGetCount() const MOMO_NOEXCEPT
		{
			return (size_t)(mState[1] & 3);
		}

		void pvSetEmpty() MOMO_NOEXCEPT
		{
			std::fill_n(mHashData.shortHashes, maxCount, (ShortHash)emptyShortHash);
			mState[0] = (uint8_t)0;
			mState[1] = (uint8_t)0;
		}

		static ShortHash pvGetShortHash(size_t hashCode) MOMO_NOEXCEPT
		{
			return (ShortHash)(hashCode >> hashCodeShift);
		}

		static size_t pvGetProbeShift(size_t logBucketCount) MOMO_NOEXCEPT
		{
			return (logBucketCount + logBucketCountAddend + 1) % logBucketCountStep;
		}

		void pvSetMaxProbe(size_t hashCode, size_t logBucketCount, size_t probe) MOMO_NOEXCEPT
		{
			size_t bucketCount = (size_t)1 << logBucketCount;
			size_t startBucketIndex = BucketBase<maxCount>::GetStartBucketIndex(hashCode,
				bucketCount);
			size_t thisBucketIndex = (startBucketIndex + probe) & (bucketCount - 1);
			BucketOpen2N2* startBucket = this - (ptrdiff_t)thisBucketIndex
				+ (ptrdiff_t)startBucketIndex;
			if (probe <= startBucket->GetMaxProbe(logBucketCount))
				return;
			if (probe <= (size_t)255)
			{
				startBucket->mState[0] = (uint8_t)probe;
				return;
			}
			size_t maxProbe0 = probe - 1;
			size_t maxProbe1 = 0;
			while (maxProbe0 >= (size_t)255)
			{
				maxProbe0 >>= 1;
				++maxProbe1;
			}
			startBucket->mState[0] = (uint8_t)maxProbe0 + 1;
			startBucket->mState[1] &= (uint8_t)3;
			startBucket->mState[1] |= (uint8_t)(maxProbe1 << 2);
		}

	private:
		uint8_t mState[2];
		HashData<maxCount, useHashCodePartGetter> mHashData;
		ObjectBuffer<Item, ItemTraits::alignment> mItems[maxCount];
	};
}

template<size_t tMaxCount = 3>
struct HashBucketOpen2N2 : public internal::HashBucketBase<tMaxCount>
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
	using Bucket = internal::BucketOpen2N2<ItemTraits, maxCount, useHashCodePartGetter>;
};

} // namespace momo

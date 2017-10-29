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
		typedef ArrayBounds<Iterator> Bounds;

		typedef BucketParamsOpen<MemManager> Params;

	private:
		template<size_t count, bool useHashCodePartGetter>
		struct HashData;

		template<size_t count>
		struct HashData<count, false>
		{
			union
			{
				uint16_t state;
				uint16_t hashes[count];
				uint8_t hashProbes[count];
			};
		};

		template<size_t count>
		struct HashData<count, true>
		{
			union
			{
				uint8_t state;
				uint8_t hashes[count];
			};
			uint8_t hashProbes[count];
		};

		typedef typename UIntSelector<useHashCodePartGetter ? 1 : 2>::UInt Hash;

		static const size_t hashCodeShift = sizeof(size_t) * 8 - sizeof(Hash) * 8 + 1;
		static const Hash emptyHash = (Hash)1 << (sizeof(Hash) * 8 - 1);
		static const Hash maskCount = 63;
		static const uint8_t emptyHashProbe = 255;

		static const size_t logBucketCountStep = 8;
		static const size_t logBucketCountAddend = 6;

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
		Iterator Find(Params& /*params*/, const Predicate& pred, size_t hashCode)
		{
			Hash hash = pvGetShortHash(hashCode);
			for (size_t i = 0; i < maxCount; ++i)
			{
				if (mHashData.hashes[i] == hash && pred(*&mItems[i]))
					return Iterator(&mItems[i] + 1);
			}
			return Iterator(nullptr);
		}

		bool IsFull() const MOMO_NOEXCEPT
		{
			return mHashData.state < emptyHash;
		}

		bool WasFull() const MOMO_NOEXCEPT
		{
			if (mHashData.state < emptyHash)
				return true;
			return (mHashData.state & (maskCount + 1)) != (Hash)0;
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
			MOMO_NOEXCEPT_IF(noexcept(itemCreator(std::declval<Item*>())))
		{
			size_t count = pvGetCount();
			MOMO_ASSERT(count < maxCount);
			Item* pitem = &mItems[maxCount - 1 - count];
			itemCreator(pitem);
			mHashData.hashes[maxCount - 1 - count] = pvGetShortHash(hashCode);
			if (useHashCodePartGetter)
			{
				uint8_t& hashProbe = mHashData.hashProbes[maxCount - 1 - count];
				size_t probeShift = pvGetProbeShift(logBucketCount);
				if (probe < ((size_t)1 << probeShift))
					hashProbe = (uint8_t)(((hashCode >> logBucketCount) << probeShift) | probe);
				else
					hashProbe = emptyHashProbe;
			}
			if (count + 1 < maxCount)
				++mHashData.state;
			return Iterator(pitem + 1);
		}

		template<typename ItemReplacer>
		Iterator Remove(Params& /*params*/, Iterator iter, const ItemReplacer& itemReplacer)
		{
			size_t count = pvGetCount();
			size_t index = std::addressof(*iter) - &mItems[0];
			MOMO_ASSERT(index >= maxCount - count);
			itemReplacer(*&mItems[maxCount - count], *&mItems[index]);
			mHashData.hashes[index] = mHashData.hashes[maxCount - count];
			mHashData.hashes[maxCount - count] = emptyHash;
			if (useHashCodePartGetter)
				mHashData.hashProbes[index] = mHashData.hashProbes[maxCount - count];
			if (count < maxCount)
				--mHashData.state;
			else
				mHashData.state = emptyHash + maskCount + (Hash)maxCount;
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
				| ((size_t)mHashData.hashes[index] << hashCodeShift);
		}

	private:
		size_t pvGetCount() const MOMO_NOEXCEPT
		{
			if (mHashData.state < emptyHash)
				return maxCount;
			return (size_t)(mHashData.state & maskCount);
		}

		void pvSetEmpty() MOMO_NOEXCEPT
		{
			std::fill_n(mHashData.hashes, maxCount, (Hash)emptyHash);
		}

		static Hash pvGetShortHash(size_t hashCode) MOMO_NOEXCEPT
		{
			return (Hash)(hashCode >> hashCodeShift);
		}

		static size_t pvGetProbeShift(size_t logBucketCount) MOMO_NOEXCEPT
		{
			return (logBucketCount + logBucketCountAddend + 1) % logBucketCountStep;
		}

	private:
		HashData<maxCount, useHashCodePartGetter> mHashData;
		ObjectBuffer<Item, ItemTraits::alignment> mItems[maxCount];
	};
}

template<size_t tMaxCount = 4>
struct HashBucketOpen2N : public internal::HashBucketBase<tMaxCount>
{
	static const size_t maxCount = tMaxCount;

	static const bool isNothrowAddableIfNothrowCreatable = true;

	static size_t CalcCapacity(size_t bucketCount) MOMO_NOEXCEPT
	{
		return (bucketCount * maxCount / 8) * 5;
	}

	static size_t GetBucketCountShift(size_t /*bucketCount*/) MOMO_NOEXCEPT
	{
		return 1;
	}

	static size_t GetNextBucketIndex(size_t bucketIndex, size_t bucketCount,
		size_t /*probe*/) MOMO_NOEXCEPT
	{
		return (bucketIndex + 1) & (bucketCount - 1);
	}

	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = internal::BucketOpen2N<ItemTraits, maxCount, useHashCodePartGetter>;
};

} // namespace momo

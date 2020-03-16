/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/details/HashBucketOpen8.h

  namespace momo:
    class HashBucketOpen8

\**********************************************************/

#pragma once

#include "HashBucketOpen2N2.h"
#include "HashBucketOpenN1.h"

namespace momo
{

#ifdef MOMO_USE_SSE2

namespace internal
{
	template<typename TItemTraits>
	class BucketOpen8 : public BucketOpenN1<TItemTraits, 7, false, int64_t>
	{
	private:
		typedef internal::BucketOpenN1<TItemTraits, 7, false, int64_t> BucketOpenN1;

	public:
		static const size_t maxCount = 7;

		using typename BucketOpenN1::Item;

		using typename BucketOpenN1::Iterator;

		using typename BucketOpenN1::Params;

	public:
		explicit BucketOpen8() noexcept
		{
		}

		BucketOpen8(const BucketOpen8&) = delete;

		~BucketOpen8() = default;

		BucketOpen8& operator=(const BucketOpen8&) = delete;

		template<typename Predicate>
		Iterator Find(Params& /*params*/, const Predicate& pred, size_t hashCode)
		{
			uint8_t shortHash = BucketOpenN1::ptCalcShortHash(hashCode);
			__m128i shortHashes = _mm_set1_epi8(static_cast<char>(shortHash));
			__m128i thisShortHashes = _mm_set_epi64x(int64_t{0}, BucketOpenN1::ptGetData());
			int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(shortHashes, thisShortHashes));
			mask &= (1 << maxCount) - 1;
			while (mask != 0)
			{
				size_t index = pvCountTrailingZeros(static_cast<uint32_t>(mask));
				Item* pitem = BucketOpenN1::ptGetItemPtr(index);
				if (pred(*pitem))
					return pitem;
				mask &= mask - 1;
			}
			return nullptr;
		}

		static size_t GetNextBucketIndex(size_t bucketIndex, size_t /*hashCode*/,
			size_t bucketCount, size_t probe) noexcept
		{
			return (bucketIndex + probe) & (bucketCount - 1);	// quadratic probing
		}

	private:
		static size_t pvCountTrailingZeros(uint32_t mask) noexcept
		{
			MOMO_ASSERT(0 < mask && mask < 128);
#ifdef MOMO_CTZ32
			return static_cast<size_t>(MOMO_CTZ32(mask));
#else
			static const uint8_t tab[127] =
			{
				   0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
				4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
				5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
				4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
				6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
				4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
				5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
				4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
			};
			return size_t{tab[mask - 1]};
#endif
		}
	};
}

class HashBucketOpen8 : public internal::HashBucketBase
{
public:
	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = typename std::conditional<
		(useHashCodePartGetter || sizeof(typename ItemTraits::Item) > 32),	//?
		internal::BucketOpen2N2<ItemTraits, 3, useHashCodePartGetter>,
		internal::BucketOpen8<ItemTraits>>::type;

public:
	static size_t CalcCapacity(size_t bucketCount, size_t bucketMaxItemCount) noexcept
	{
		double maxItemCount = static_cast<double>(bucketCount * bucketMaxItemCount);
		if (bucketMaxItemCount == 7)
			return static_cast<size_t>(maxItemCount / 14.0 * 13.0);	// BucketOpen8
		else
			return static_cast<size_t>(maxItemCount / 12.0 * 11.0);	// BucketOpen2N2
	}

	static size_t GetBucketCountShift(size_t /*bucketCount*/,
		size_t /*bucketMaxItemCount*/) noexcept
	{
		return 1;
	}
};

#else

typedef HashBucketOpen2N2<3> HashBucketOpen8;

#endif

} // namespace momo

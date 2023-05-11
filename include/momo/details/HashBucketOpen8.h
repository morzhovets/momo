/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/details/HashBucketOpen8.h

  namespace momo:
    class HashBucketOpen8

\**********************************************************/

#pragma once

#include "HashBucketOpen2N2.h"
#include "HashBucketOpenN1.h"

#ifdef MOMO_USE_SSE2
#include <emmintrin.h>
#endif

namespace momo
{

namespace internal
{
	template<typename TItemTraits>
	class BucketOpen8 : public BucketOpenN1<TItemTraits, 7, false>
	{
	private:
		typedef internal::BucketOpenN1<TItemTraits, 7, false> BucketOpenN1;

	public:
		static const size_t maxCount = 7;

		using typename BucketOpenN1::Item;

		using typename BucketOpenN1::Iterator;

		using typename BucketOpenN1::Params;

	public:
		explicit BucketOpen8() noexcept = default;

		BucketOpen8(const BucketOpen8&) = delete;

		~BucketOpen8() noexcept = default;

		BucketOpen8& operator=(const BucketOpen8&) = delete;

		template<bool first, std::predicate<const Item&> Predicate>
		MOMO_FORCEINLINE Iterator Find(Params& /*params*/, Predicate pred, size_t hashCode)
		{
			return pvFind<first>(pred, hashCode);
		}

		static size_t GetNextBucketIndex(size_t bucketIndex, size_t /*hashCode*/,
			size_t bucketCount, size_t probe) noexcept
		{
			return (bucketIndex + probe) & (bucketCount - 1);	// quadratic probing
		}

	private:
		template<bool first, typename Predicate>
		MOMO_FORCEINLINE Iterator pvFind(Predicate pred, size_t hashCode)
		{
			static_assert(std::endian::native == std::endian::little);
#ifdef MOMO_PREFETCH
			if constexpr (first)
				MOMO_PREFETCH(BucketOpenN1::ptGetItemPtr(3));
#endif
			uint8_t shortHash = BucketOpenN1::ptCalcShortHash(hashCode);
#ifdef MOMO_USE_SSE2
			__m128i shortHashes = _mm_set1_epi8(static_cast<char>(shortHash));
			__m128i thisShortHashes = _mm_set_epi64x(int64_t{0},
				MemCopyer::FromBuffer<int64_t>(BucketOpenN1::ptGetData()));
			int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(shortHashes, thisShortHashes));
			mask &= (1 << maxCount) - 1;
			for (; mask != 0; mask &= mask - 1)
			{
				size_t index = static_cast<size_t>(std::countr_zero(static_cast<uint8_t>(mask)));
				Item* itemPtr = BucketOpenN1::ptGetItemPtr(index);
				if (pred(std::as_const(*itemPtr))) [[likely]]
					return itemPtr;
			}
#else
			uint64_t thisShortHashes = MemCopyer::FromBuffer<uint64_t>(BucketOpenN1::ptGetData());
			uint64_t xorHashes = (shortHash * 0x0101010101010101ull) ^ thisShortHashes;
			uint64_t mask = (xorHashes - 0x0101010101010101ull) & ~xorHashes & 0x0080808080808080ull;
			for (; mask != 0; mask &= mask - 1)
			{
				size_t index = static_cast<size_t>(std::countr_zero(mask)) >> 3;
				Item* itemPtr = BucketOpenN1::ptGetItemPtr(index);
				if (pred(std::as_const(*itemPtr))) [[likely]]
					return itemPtr;
			}
#endif
			return nullptr;
		}
	};
}

class HashBucketOpen8 : public internal::HashBucketBase
{
public:
	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = std::conditional_t<(std::endian::native != std::endian::little
		|| useHashCodePartGetter || sizeof(typename ItemTraits::Item) > 32),	//?
		internal::BucketOpen2N2<ItemTraits, 3, useHashCodePartGetter>,
		internal::BucketOpen8<ItemTraits>>;

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

} // namespace momo
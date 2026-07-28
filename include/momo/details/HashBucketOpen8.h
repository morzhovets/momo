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
# include <emmintrin.h>
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

#ifdef MOMO_USE_SSE2
		typedef __m128i PreparedCode;
#else
		typedef uint64_t PreparedCode;
#endif

	public:
		explicit BucketOpen8() noexcept = default;

		BucketOpen8(const BucketOpen8&) = delete;

		~BucketOpen8() noexcept = default;

		BucketOpen8& operator=(const BucketOpen8&) = delete;

		static PreparedCode PrepareFind(size_t hashCode) noexcept
		{
			uint8_t shortCode = BucketOpenN1::ptCalcShortCode(hashCode);
#ifdef MOMO_USE_SSE2
			return _mm_set1_epi8(static_cast<char>(shortCode));
#else
			return uint64_t{shortCode} * 0x0101010101010101ull;
#endif
		}

		template<bool first, conceptObjectPredicate<Item> ItemPredicate>
		MOMO_FORCEINLINE Iterator Find(Params& /*params*/,
			FastCopyableFunctor<ItemPredicate> itemPred, PreparedCode prepCode)
		{
#ifdef MOMO_PREFETCH
			if constexpr (first && 8 + 5 * sizeof(Item) >= std::hardware_destructive_interference_size)
				MOMO_PREFETCH(PtrCaster::ToBytePtr(this) + std::hardware_destructive_interference_size);
#endif
			return pvFind(itemPred, prepCode);
		}

		static size_t GetNextBucketIndex(size_t bucketIndex, size_t /*hashCode*/,
			size_t bucketCount, size_t probe) noexcept
		{
			return (bucketIndex + probe) & (bucketCount - 1);	// quadratic probing
		}

	private:
		template<conceptObjectPredicate<Item> ItemPredicate>
		MOMO_FORCEINLINE Iterator pvFind(FastCopyableFunctor<ItemPredicate> itemPred,
			PreparedCode prepCode)
		{
			static_assert(std::endian::native == std::endian::little);
#ifdef MOMO_USE_SSE2
			static const size_t maskIndexShift = 0;
#else
			static const size_t maskIndexShift = 3;
#endif
			auto mask = pvFindCode(prepCode);
			for (; mask != 0; mask &= mask - 1)
			{
				size_t index = static_cast<size_t>(std::countr_zero(mask)) >> maskIndexShift;
				Item* itemPtr = BucketOpenN1::ptGetItemPtr(index);
				if (itemPred(std::as_const(*itemPtr))) [[likely]]
					return itemPtr;
			}
			return nullptr;
		}

#ifdef MOMO_USE_SSE2
		MOMO_FORCEINLINE uint8_t pvFindCode(PreparedCode prepCode) noexcept
		{
			__m128i shortCodes = _mm_set_epi64x(int64_t{0},
				MemCopyer::FromBuffer<int64_t>(BucketOpenN1::ptGetData()));
			int mask = _mm_movemask_epi8(_mm_cmpeq_epi8(prepCode, shortCodes));
			mask &= (1 << maxCount) - 1;
			return static_cast<uint8_t>(mask);
		}
#else
		MOMO_FORCEINLINE uint64_t pvFindCode(PreparedCode prepCode) noexcept
		{
			uint64_t shortCodes = MemCopyer::FromBuffer<uint64_t>(BucketOpenN1::ptGetData());
			uint64_t xorCodes = prepCode ^ shortCodes;
			return (xorCodes - 0x0101010101010101ull) & ~xorCodes & 0x0080808080808080ull;
		}
#endif
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

/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/details/HashBucketOpen8.h

  namespace momo:
    struct HashBucketOpen8

\**********************************************************/

#pragma once

#include "HashBucketOpen2N2.h"
#include "HashBucketOpenN1.h"

namespace momo
{

#ifdef MOMO_USE_SSE2

struct HashBucketOpen8 : public internal::HashBucketBase
{
	static size_t CalcCapacity(size_t bucketCount, size_t bucketMaxItemCount) MOMO_NOEXCEPT
	{
		if (bucketMaxItemCount == 7)
			return bucketCount * 6;
		else
			return (bucketCount * bucketMaxItemCount / 6) * 5;
	}

	static size_t GetBucketCountShift(size_t /*bucketCount*/,
		size_t /*bucketMaxItemCount*/) MOMO_NOEXCEPT
	{
		return 1;
	}

	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = typename std::conditional<
		useHashCodePartGetter /*|| sizeof(typename ItemTraits::Item) >= 32*/,
		internal::BucketOpen2N2<ItemTraits, 3, useHashCodePartGetter>,
		internal::BucketOpenN1<ItemTraits, true, 7, false>>::type;
};

#else

typedef HashBucketOpen2N2<3> HashBucketOpen8;

#endif

} // namespace momo

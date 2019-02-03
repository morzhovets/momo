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

class HashBucketOpen8 : public internal::HashBucketBase
{
public:
	template<typename ItemTraits, bool useHashCodePartGetter>
	using Bucket = typename std::conditional<
		(useHashCodePartGetter || sizeof(typename ItemTraits::Item) > 32),	//?
		internal::BucketOpen2N2<ItemTraits, 3, useHashCodePartGetter>,
		internal::BucketOpenN1<ItemTraits, true, 7, false>>::type;

public:
	static size_t CalcCapacity(size_t bucketCount, size_t bucketMaxItemCount) noexcept
	{
		if (bucketMaxItemCount == 7)
			return bucketCount * 6;
		else
			return (bucketCount * bucketMaxItemCount / 6) * 5;
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

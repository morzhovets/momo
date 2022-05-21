/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/stdish/vector.h

  namespace momo::stdish:
    class segmented_vector
    class segmented_vector_sqrt
    class merge_vector

\**********************************************************/

#pragma once

#include "vector.h"
#include "../SegmentedArray.h"
#include "../MergeArray.h"

namespace momo::stdish
{

template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using segmented_vector = vector<TValue, TAllocator,
	SegmentedArray<TValue, MemManagerStd<TAllocator>>>;

template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using segmented_vector_sqrt = vector<TValue, TAllocator,
	SegmentedArraySqrt<TValue, MemManagerStd<TAllocator>>>;

template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using merge_vector = vector<TValue, TAllocator,
	MergeArray<TValue, MemManagerStd<TAllocator>>>;

} // namespace momo::stdish

/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
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
class segmented_vector
	: public vector_adaptor<SegmentedArray<TValue, MemManagerStd<TAllocator>>>
{
private:
	typedef vector_adaptor<SegmentedArray<TValue, MemManagerStd<TAllocator>>> VectorAdaptor;

public:
	using VectorAdaptor::VectorAdaptor;

	using VectorAdaptor::operator=;

	friend void swap(segmented_vector& left, segmented_vector& right) noexcept
	{
		left.swap(right);
	}
};

template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
class segmented_vector_sqrt
	: public vector_adaptor<SegmentedArraySqrt<TValue, MemManagerStd<TAllocator>>>
{
private:
	typedef vector_adaptor<SegmentedArraySqrt<TValue, MemManagerStd<TAllocator>>> VectorAdaptor;

public:
	using VectorAdaptor::VectorAdaptor;

	using VectorAdaptor::operator=;

	friend void swap(segmented_vector_sqrt& left, segmented_vector_sqrt& right) noexcept
	{
		left.swap(right);
	}
};

template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
class merge_vector
	: public vector_adaptor<MergeArray<TValue, MemManagerStd<TAllocator>>>
{
private:
	typedef vector_adaptor<MergeArray<TValue, MemManagerStd<TAllocator>>> VectorAdaptor;

public:
	using VectorAdaptor::VectorAdaptor;

	using VectorAdaptor::operator=;

	friend void swap(merge_vector& left, merge_vector& right) noexcept
	{
		left.swap(right);
	}
};

} // namespace momo::stdish

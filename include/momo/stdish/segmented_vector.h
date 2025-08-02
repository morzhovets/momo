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
class MOMO_EMPTY_BASES segmented_vector
	: public vector_adaptor<SegmentedArray<TValue, MemManagerStd<TAllocator>>>,
	public momo::internal::Swappable<segmented_vector>
{
private:
	typedef vector_adaptor<SegmentedArray<TValue, MemManagerStd<TAllocator>>> VectorAdaptor;

public:
	using VectorAdaptor::VectorAdaptor;

	using VectorAdaptor::operator=;
};

template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
class MOMO_EMPTY_BASES segmented_vector_sqrt
	: public vector_adaptor<SegmentedArraySqrt<TValue, MemManagerStd<TAllocator>>>,
	public momo::internal::Swappable<segmented_vector_sqrt>
{
private:
	typedef vector_adaptor<SegmentedArraySqrt<TValue, MemManagerStd<TAllocator>>> VectorAdaptor;

public:
	using VectorAdaptor::VectorAdaptor;

	using VectorAdaptor::operator=;
};

template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
class MOMO_EMPTY_BASES merge_vector
	: public vector_adaptor<MergeArray<TValue, MemManagerStd<TAllocator>>>,
	public momo::internal::Swappable<merge_vector>
{
private:
	typedef vector_adaptor<MergeArray<TValue, MemManagerStd<TAllocator>>> VectorAdaptor;

public:
	using VectorAdaptor::VectorAdaptor;

	using VectorAdaptor::operator=;
};

#define MOMO_DECLARE_DEDUCTION_GUIDES(vector) \
template<typename Value, \
	momo::internal::conceptAllocator Allocator = std::allocator<Value>> \
vector(size_t, Value, Allocator = Allocator()) \
	-> vector<Value, Allocator>; \
template<typename Iterator, \
	typename Value = std::iter_value_t<Iterator>, \
	momo::internal::conceptAllocator Allocator = std::allocator<Value>> \
vector(Iterator, Iterator, Allocator = Allocator()) \
	-> vector<Value, Allocator>; \
template<typename Value, \
	momo::internal::conceptAllocator Allocator = std::allocator<Value>> \
vector(std::initializer_list<Value>, Allocator = Allocator()) \
	-> vector<Value, Allocator>; \
template<typename Value, typename Allocator> \
vector(vector<Value, Allocator>, std::type_identity_t<Allocator>) \
	-> vector<Value, Allocator>;

#define MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(vector) \
template<std::ranges::input_range Range, \
	typename Value = std::ranges::range_value_t<Range>, \
	momo::internal::conceptAllocator Allocator = std::allocator<Value>> \
vector(std::from_range_t, Range&&, Allocator = Allocator()) \
	-> vector<Value, Allocator>;

MOMO_DECLARE_DEDUCTION_GUIDES(segmented_vector)
MOMO_DECLARE_DEDUCTION_GUIDES(segmented_vector_sqrt)
MOMO_DECLARE_DEDUCTION_GUIDES(merge_vector)

#if defined(__cpp_lib_containers_ranges)
MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(segmented_vector)
MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(segmented_vector_sqrt)
MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(merge_vector)
#endif

#undef MOMO_DECLARE_DEDUCTION_GUIDES
#undef MOMO_DECLARE_DEDUCTION_GUIDES_RANGES

} // namespace momo::stdish

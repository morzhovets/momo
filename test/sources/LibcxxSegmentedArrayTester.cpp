/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxSegmentedArrayTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_ARRAY

#include "LibcxxTester.h"

#include "../../include/momo/stdish/vector.h"
#include "../../include/momo/SegmentedArray.h"

namespace
{

using namespace libcxx_insert_range_seq;
using namespace libcxx_from_range_seq;

namespace libcxx_segmented_array_sqrt
{

class LibcxxSegmentedArraySettings
	: public momo::SegmentedArraySettings<momo::SegmentedArrayItemCountFunc::sqrt, 3>
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};

LIBCXX_NAMESPACE_STD_BEGIN
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::vector<TValue, TAllocator,
	momo::SegmentedArray<TValue, momo::MemManagerStd<TAllocator>,
		momo::SegmentedArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>,
		LibcxxSegmentedArraySettings>>;
LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_FAILURE
#define LIBCXX_TEST_SEGMENTED_ARRAY
#define LIBCXX_TEST_PREFIX "libcxx_segmented_array_sqrt"
#include LIBCXX_HEADER(VectorTests.h)
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_SEGMENTED_ARRAY
#undef LIBCXX_TEST_FAILURE

} // namespace libcxx_segmented_array_sqrt

namespace libcxx_segmented_array_cnst
{

class LibcxxSegmentedArraySettings
	: public momo::SegmentedArraySettings<momo::SegmentedArrayItemCountFunc::cnst, 0>
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};

LIBCXX_NAMESPACE_STD_BEGIN
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::vector<TValue, TAllocator,
	momo::SegmentedArray<TValue, momo::MemManagerStd<TAllocator>,
		momo::SegmentedArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>,
		LibcxxSegmentedArraySettings>>;
LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_FAILURE
#define LIBCXX_TEST_SEGMENTED_ARRAY
#define LIBCXX_TEST_PREFIX "libcxx_segmented_array_cnst"
#include LIBCXX_HEADER(VectorTests.h)
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_SEGMENTED_ARRAY
#undef LIBCXX_TEST_FAILURE

} // namespace libcxx_segmented_array_cnst

} // namespace

#endif // TEST_LIBCXX_ARRAY

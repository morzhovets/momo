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

#undef _LIBCPP_DEBUG
#undef _LIBCPP_DEBUG_LEVEL
#define _LIBCPP_DEBUG 1
#define _LIBCPP_DEBUG_LEVEL 1

#define LIBCXX_TEST_SEGMENTED_ARRAY

#define LIBCXX_TEST_PREFIX "libcxx_test_segmented_array_sqrt"
namespace libcxx_test_segmented_array_sqrt
{
class LibcxxSegmentedArraySettings
	: public momo::SegmentedArraySettings<momo::SegmentedArrayItemCountFunc::sqrt, 3>
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::vector<TValue, TAllocator,
	momo::SegmentedArray<TValue, momo::MemManagerStd<TAllocator>,
		momo::SegmentedArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>,
		LibcxxSegmentedArraySettings>>;
#include "libcxx/VectorTests.h"
}
#undef LIBCXX_TEST_PREFIX

#define LIBCXX_TEST_PREFIX "libcxx_test_segmented_array_cnst"
namespace libcxx_test_segmented_array_cnst
{
class LibcxxSegmentedArraySettings
	: public momo::SegmentedArraySettings<momo::SegmentedArrayItemCountFunc::cnst, 0>
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::vector<TValue, TAllocator,
	momo::SegmentedArray<TValue, momo::MemManagerStd<TAllocator>,
		momo::SegmentedArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>,
		LibcxxSegmentedArraySettings>>;
#include "libcxx/VectorTests.h"
}
#undef LIBCXX_TEST_PREFIX

#undef LIBCXX_TEST_SEGMENTED_ARRAY

#undef _LIBCPP_DEBUG
#undef _LIBCPP_DEBUG_LEVEL

#endif // TEST_LIBCXX_ARRAY

/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxSegmentedArrayTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_ARRAY

#include "LibcxxTester.h"

#include "../../include/momo/stdish/segmented_vector.h"

namespace
{

using namespace libcxx_insert_range_seq;
using namespace libcxx_from_range_seq;

namespace libcxx_test_segmented_array_sqrt_0
{

class LibcxxSegmentedArraySettings
	: public momo::SegmentedArraySettings<momo::SegmentedArrayItemCountFunc::sqrt, 0>
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};

namespace std
{
	using namespace ::std;

	template<typename TValue,
		typename TAllocator = std::allocator<TValue>>
	using vector = momo::stdish::vector<TValue, TAllocator,
		momo::SegmentedArray<TValue, momo::MemManagerStd<TAllocator>,
			momo::SegmentedArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>,
			LibcxxSegmentedArraySettings>>;
}

#define LIBCXX_TEST_FAILURE
#define LIBCXX_TEST_SEGMENTED_ARRAY
#define LIBCXX_TEST_PREFIX "libcxx_test_segmented_array_sqrt_0"
#include "libcxx/VectorTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_SEGMENTED_ARRAY
#undef LIBCXX_TEST_FAILURE

} // namespace libcxx_test_segmented_array_sqrt_0

namespace libcxx_test_segmented_array_cnst_0
{

class LibcxxSegmentedArraySettings
	: public momo::SegmentedArraySettings<momo::SegmentedArrayItemCountFunc::cnst, 0>
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};

namespace std
{
	using namespace ::std;

	template<typename TValue,
		typename TAllocator = std::allocator<TValue>>
	using vector = momo::stdish::vector<TValue, TAllocator,
		momo::SegmentedArray<TValue, momo::MemManagerStd<TAllocator>,
			momo::SegmentedArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>,
			LibcxxSegmentedArraySettings>>;
}

#define LIBCXX_TEST_FAILURE
#define LIBCXX_TEST_SEGMENTED_ARRAY
#define LIBCXX_TEST_PREFIX "libcxx_test_segmented_array_cnst_0"
#include "libcxx/VectorTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_SEGMENTED_ARRAY
#undef LIBCXX_TEST_FAILURE

} // namespace libcxx_test_segmented_array_cnst_0

namespace libcxx_test_segmented_array_cnst
{

namespace std
{
	using namespace ::std;

	template<typename TValue,
		typename TAllocator = std::allocator<TValue>>
	using vector = momo::stdish::segmented_vector<TValue, TAllocator>;
}

#define LIBCXX_TEST_SEGMENTED_ARRAY
#define LIBCXX_TEST_PREFIX "libcxx_test_segmented_array_cnst"
#include "libcxx/VectorTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_SEGMENTED_ARRAY

} // namespace libcxx_test_segmented_array_cnst

namespace libcxx_test_segmented_array_sqrt
{

namespace std
{
	using namespace ::std;

	template<typename TValue,
		typename TAllocator = std::allocator<TValue>>
	using vector = momo::stdish::segmented_vector_sqrt<TValue, TAllocator>;
}

#define LIBCXX_TEST_SEGMENTED_ARRAY
#define LIBCXX_TEST_PREFIX "libcxx_test_segmented_array_sqrt"
#include "libcxx/VectorTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_SEGMENTED_ARRAY

} // namespace libcxx_test_segmented_array_sqrt

} // namespace

#endif // TEST_LIBCXX_ARRAY

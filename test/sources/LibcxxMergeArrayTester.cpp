/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxMergeArrayTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_ARRAY

#include "LibcxxTester.h"

#include "../../include/momo/stdish/segmented_vector.h"

#undef _LIBCPP_DEBUG
#undef _LIBCPP_DEBUG_LEVEL
#define _LIBCPP_DEBUG 1
#define _LIBCPP_DEBUG_LEVEL 1

namespace libcxx_test_merge_array_0
{

class LibcxxMergeArraySettings : public momo::MergeArraySettings<0>
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
		momo::MergeArray<TValue, momo::MemManagerStd<TAllocator>,
			momo::MergeArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>,
			LibcxxMergeArraySettings>>;
}

using std::vector;

#define LIBCXX_TEST_FAILURE
#define LIBCXX_TEST_SEGMENTED_ARRAY
#define LIBCXX_TEST_PREFIX "libcxx_test_merge_array_0"
#include "libcxx/VectorTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_SEGMENTED_ARRAY
#undef LIBCXX_TEST_FAILURE

} // namespace libcxx_test_merge_array_0

#undef _LIBCPP_DEBUG
#undef _LIBCPP_DEBUG_LEVEL

namespace libcxx_test_merge_array
{

namespace std
{
	using namespace ::std;

	template<typename TValue,
		typename TAllocator = std::allocator<TValue>>
	using vector = momo::stdish::merge_vector<TValue, TAllocator>;
}

using std::vector;

#define LIBCXX_TEST_SEGMENTED_ARRAY
#define LIBCXX_TEST_PREFIX "libcxx_test_merge_array"
#include "libcxx/VectorTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_SEGMENTED_ARRAY

} // namespace libcxx_test_merge_array

#endif // TEST_LIBCXX_ARRAY

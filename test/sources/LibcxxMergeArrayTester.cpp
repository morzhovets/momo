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

namespace
{

using namespace libcxx_insert_range_seq;
using namespace libcxx_from_range_seq;

namespace libcxx_merge_array_0
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
	using vector = momo::stdish::vector_adaptor<momo::MergeArrayCore<
		momo::MergeArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>,
		LibcxxMergeArraySettings>>;
}

#define LIBCXX_TEST_FAILURE
#define LIBCXX_TEST_SEGMENTED_ARRAY
#define LIBCXX_TEST_PREFIX "merge_array_0"
#include "libcxx/VectorTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_SEGMENTED_ARRAY
#undef LIBCXX_TEST_FAILURE

} // namespace libcxx_merge_array_0

namespace libcxx_merge_array
{

namespace std
{
	using namespace ::std;

	template<typename TValue,
		typename TAllocator = std::allocator<TValue>>
	using vector = momo::stdish::merge_vector<TValue, TAllocator>;
}

#define LIBCXX_TEST_SEGMENTED_ARRAY
#define LIBCXX_TEST_CLASS momo::stdish::merge_vector
#define LIBCXX_TEST_PREFIX "merge_array"
#include "libcxx/VectorTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_CLASS
#undef LIBCXX_TEST_SEGMENTED_ARRAY

} // namespace libcxx_merge_array

} // namespace

#endif // TEST_LIBCXX_ARRAY

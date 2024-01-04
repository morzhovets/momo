/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxArrayTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_ARRAY

#include "LibcxxTester.h"

#include "../../include/momo/stdish/vector.h"

namespace libcxx_test_array_0
{

class LibcxxArraySettings : public momo::ArraySettings<0, false>
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};

LIBCXX_NAMESPACE_STD_BEGIN
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::vector<TValue, TAllocator,
	momo::Array<TValue, momo::MemManagerStd<TAllocator>,
		momo::ArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>, LibcxxArraySettings>>;
LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_FAILURE
#define LIBCXX_TEST_ARRAY
#define LIBCXX_TEST_PREFIX "libcxx_test_array_0"
#include LIBCXX_HEADER(VectorTests.h)
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_ARRAY
#undef LIBCXX_TEST_FAILURE

} // namespace libcxx_test_array_0

namespace libcxx_test_array_5
{

LIBCXX_NAMESPACE_STD_BEGIN
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::vector_intcap<5, TValue, TAllocator>;
LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_INTCAP_ARRAY
#define LIBCXX_TEST_PREFIX "libcxx_test_array_5"
#include LIBCXX_HEADER(VectorTests.h)
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_INTCAP_ARRAY

} // namespace libcxx_test_array_5

#endif // TEST_LIBCXX_ARRAY

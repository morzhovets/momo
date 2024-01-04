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

#undef _LIBCPP_DEBUG
#undef _LIBCPP_DEBUG_LEVEL
#define _LIBCPP_DEBUG 1
#define _LIBCPP_DEBUG_LEVEL 1

#define LIBCXX_TEST_ARRAY
#define LIBCXX_TEST_PREFIX "libcxx_test_array_0"
namespace libcxx_test_array_0
{
class LibcxxArraySettings : public momo::ArraySettings<0, false>
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::vector<TValue, TAllocator,
	momo::Array<TValue, momo::MemManagerStd<TAllocator>,
		momo::ArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>, LibcxxArraySettings>>;
#include "libcxx/VectorTests.h"
}
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_ARRAY

#undef _LIBCPP_DEBUG
#undef _LIBCPP_DEBUG_LEVEL

#define LIBCXX_TEST_INTCAP_ARRAY
#define LIBCXX_TEST_PREFIX "libcxx_test_array_5"
namespace libcxx_test_array_5
{
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::vector_intcap<5, TValue, TAllocator>;
#include "libcxx/VectorTests.h"
}
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_INTCAP_ARRAY

#endif // TEST_LIBCXX_ARRAY

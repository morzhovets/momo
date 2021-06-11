/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxHashSetTesterOpen8.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_SET

#include "../../momo/details/HashBucketOpen8.h"

//#define LIBCXX_TEST_BUCKET momo::HashBucketOpen8
//#define LIBCXX_TEST_BUCKET_NAME "open8"

//#include "LibcxxHashSetTester.h"

#include "LibcxxTester.h"

#include "../../momo/stdish/unordered_set.h"

namespace
{

#define LIBCXX_TEST_PREFIX "libcxx_test_hash_set_open"
template<typename TKey,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<TKey>>
using unordered_set = momo::stdish::unordered_set_open<TKey, THashFunc, TEqualFunc, TAllocator>;
#include "LibcxxUnorderedSetTests.h"
#undef LIBCXX_TEST_PREFIX

} // namespace

#endif // TEST_LIBCXX_HASH_SET

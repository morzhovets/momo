/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxHashSetTesterOpen8.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_SET

#include "../../include/momo/details/HashBucketOpen8.h"

//#define LIBCXX_TEST_BUCKET momo::HashBucketOpen8
//#define LIBCXX_TEST_BUCKET_NAME "open8"

//#include "LibcxxHashSetTester.h"

#include "LibcxxTester.h"

#include "../../include/momo/stdish/unordered_set.h"

namespace
{

namespace libcxx_test_hash_set
{

namespace std
{
	using namespace ::std;

	template<typename TKey,
		typename THashFunc = std::hash<TKey>,
		typename TEqualFunc = std::equal_to<TKey>,
		typename TAllocator = std::allocator<TKey>>
	using unordered_set = momo::stdish::unordered_set_open<TKey, THashFunc, TEqualFunc, TAllocator>;
}

#define LIBCXX_TEST_PREFIX "libcxx_test_hash_set_open"
#include "libcxx/UnorderedSetTests.h"
#undef LIBCXX_TEST_PREFIX

} // namespace libcxx_test_hash_set

} // namespace

#endif // TEST_LIBCXX_HASH_SET

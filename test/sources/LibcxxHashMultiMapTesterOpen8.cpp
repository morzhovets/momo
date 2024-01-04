/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxHashMultiMapTesterOpen8.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_MULTI_MAP

#include "../../include/momo/details/HashBucketOpen8.h"

//#define LIBCXX_TEST_BUCKET momo::HashBucketOpen8
//#define LIBCXX_TEST_BUCKET_NAME "open8"

//#include "LibcxxHashMultiMapTester.h"

#include "LibcxxTester.h"

#include "../../include/momo/stdish/unordered_multimap.h"

namespace
{

namespace libcxx_test_hash_multimap
{

template<typename TKey, typename TMapped,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using unordered_multimap = momo::stdish::unordered_multimap_open<TKey, TMapped,
	THashFunc, TEqualFunc, TAllocator>;

#define LIBCXX_TEST_PREFIX "libcxx_test_hash_multimap_open"
#include "libcxx/UnorderedMultiMapTests.h"
#undef LIBCXX_TEST_PREFIX

} // namespace libcxx_test_hash_multimap

} // namespace

#endif // TEST_LIBCXX_HASH_MULTI_MAP

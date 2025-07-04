/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxHashSetTesterOpen8.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_SET

#include "../../include/momo/details/HashBucketOpen8.h"

//#define LIBCXX_TEST_BUCKET momo::HashBucketOpen8
//#define LIBCXX_TEST_PREFIX_TAIL "open8"

//#include "LibcxxHashSetTester.h"

#include "LibcxxTester.h"

#include "../../include/momo/stdish/unordered_set.h"

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_unord;

namespace libcxx_test_hash_set
{

LIBCXX_NAMESPACE_STD_BEGIN
template<typename TKey,
	typename THasher = std::hash<TKey>,
	typename TEqualComparer = std::equal_to<TKey>,
	typename TAllocator = std::allocator<TKey>>
using unordered_set = momo::stdish::unordered_set_open<TKey, THasher, TEqualComparer, TAllocator>;
LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_PREFIX "libcxx_test_hash_set_open"
#include LIBCXX_HEADER(UnorderedSetTests.h)
#undef LIBCXX_TEST_PREFIX

} // namespace libcxx_test_hash_set

} // namespace

#endif // TEST_LIBCXX_HASH_SET

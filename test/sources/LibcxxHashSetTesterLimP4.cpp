/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxHashSetTesterLimP4.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_SET

#include "../../include/momo/details/HashBucketLimP4.h"

//#define LIBCXX_TEST_BUCKET momo::HashBucketLimP4<>
//#define LIBCXX_TEST_PREFIX_TAIL "limp4"

//#include "LibcxxHashSetTester.h"

#include "LibcxxTester.h"

#include "../../include/momo/stdish/unordered_set.h"

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_unord;

namespace libcxx_test_hash_set
{

namespace std
{
	using namespace ::std;

	template<typename TKey,
		typename THasher = std::hash<TKey>,
		typename TEqualComparer = std::equal_to<TKey>,
		typename TAllocator = std::allocator<TKey>>
	using unordered_set = momo::stdish::unordered_set<TKey, THasher, TEqualComparer, TAllocator>;
}

#define LIBCXX_TEST_DEDUCT_CLASS momo::stdish::unordered_set
#define LIBCXX_TEST_PREFIX "libcxx_test_hash_set_limp4"
#include "libcxx/UnorderedSetTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_DEDUCT_CLASS

} // namespace libcxx_test_hash_set

} // namespace

#endif // TEST_LIBCXX_HASH_SET

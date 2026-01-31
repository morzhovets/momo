/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxHashListSetTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_LIST_SET

#include "LibcxxTester.h"

#include "../../include/momo/stdish/unordered_set.h"

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_unord;

namespace libcxx_hash_list_set
{

namespace std
{
	using namespace ::std;

	template<typename TKey,
		typename THasher = std::hash<TKey>,
		typename TEqualComparer = std::equal_to<TKey>,
		typename TAllocator = std::allocator<TKey>>
	using unordered_set = momo::stdish::ordered_set<TKey, THasher, TEqualComparer, TAllocator>;
}

#define LIBCXX_TEST_HASH_LIST_SET
#define LIBCXX_TEST_CLASS momo::stdish::ordered_set
#define LIBCXX_TEST_PREFIX "hash_list_set"
#include "libcxx20/UnorderedSetTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_CLASS
#undef LIBCXX_TEST_HASH_LIST_SET

} // namespace libcxx_hash_list_set

} // namespace

#endif // TEST_LIBCXX_HASH_LIST_SET

/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxHashListMapTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_LIST_MAP

#include "LibcxxTester.h"

#include "../../include/momo/stdish/unordered_map.h"

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_unord;

namespace libcxx_hash_list_map
{

namespace std
{
	using namespace ::std;

	template<typename TKey, typename TMapped,
		typename THasher = std::hash<TKey>,
		typename TEqualComparer = std::equal_to<TKey>,
		typename TAllocator = std::allocator<TKey>>
	using unordered_map = momo::stdish::ordered_map<TKey, TMapped, THasher, TEqualComparer, TAllocator>;
}

#define LIBCXX_TEST_HASH_LIST_MAP
#define LIBCXX_TEST_CLASS momo::stdish::ordered_map
#define LIBCXX_TEST_PREFIX "hash_list_map"
#include "libcxx20/UnorderedMapTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_CLASS
#undef LIBCXX_TEST_HASH_LIST_MAP

} // namespace libcxx_hash_list_map

} // namespace

#endif // TEST_LIBCXX_HASH_LIST_MAP

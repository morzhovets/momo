/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxHashMapTester_Open8.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_MAP

#include "../../include/momo/details/HashBucketOpen8.h"

//#define LIBCXX_TEST_BUCKET momo::HashBucketOpen8
//#define LIBCXX_TEST_PREFIX_TAIL "open8"

//#include "LibcxxHashMapTester.h"

#include "LibcxxTester.h"

#include "../../include/momo/stdish/unordered_map.h"

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_unord;

namespace libcxx_hash_map
{

LIBCXX_NAMESPACE_STD_BEGIN
template<typename TKey, typename TMapped,
	typename THasher = std::hash<TKey>,
	typename TEqualComparer = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using unordered_map = momo::stdish::unordered_map_open<TKey, TMapped,
	THasher, TEqualComparer, TAllocator>;
LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_PREFIX "libcxx_hash_map_open"
#include LIBCXX_HEADER(UnorderedMapTests.h)
#undef LIBCXX_TEST_PREFIX

} // namespace libcxx_hash_map

} // namespace

#endif // TEST_LIBCXX_HASH_MAP

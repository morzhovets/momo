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

#ifdef TEST_LIBCXX_NEW
# include "../../include/momo/stdish/set.h"
#endif

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_unord;

namespace libcxx_test_hash_multimap
{

LIBCXX_NAMESPACE_STD_BEGIN

template<typename TKey, typename TMapped,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using unordered_multimap = momo::stdish::unordered_multimap_open<TKey, TMapped,
	THashFunc, TEqualFunc, TAllocator>;

#ifdef TEST_LIBCXX_NEW
template<typename TKey>
using set = momo::stdish::set<TKey>;
template<typename TKey>
using multiset = momo::stdish::multiset<TKey>;
#endif

LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_PREFIX "libcxx_test_hash_multimap_open"
#include LIBCXX_HEADER(UnorderedMultiMapTests.h)
#undef LIBCXX_TEST_PREFIX

} // namespace libcxx_test_hash_multimap

} // namespace

#endif // TEST_LIBCXX_HASH_MULTI_MAP

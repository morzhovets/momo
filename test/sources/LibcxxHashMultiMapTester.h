/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxHashMultiMapTester.h

\**********************************************************/

#pragma once

#include "LibcxxTester.h"

#include "../../include/momo/stdish/unordered_multimap.h"

#ifdef TEST_LIBCXX_NEW
# include "../../include/momo/stdish/set.h"
#endif

namespace
{

namespace libcxx_test_hash_multimap
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_unord;

class LibcxxHashMultiMapSettings : public momo::HashMultiMapSettings
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};

LIBCXX_NAMESPACE_STD_BEGIN

template<typename TKey, typename TMapped,
	typename THasher = std::hash<TKey>,
	typename TEqualComparer = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using unordered_multimap = momo::stdish::unordered_multimap<TKey, TMapped, THasher, TEqualComparer, TAllocator,
	momo::HashMultiMap<TKey, TMapped, momo::HashTraitsStd<TKey, THasher, TEqualComparer, LIBCXX_TEST_BUCKET>,
		momo::MemManagerStd<TAllocator>,
		momo::HashMultiMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>>,
		LibcxxHashMultiMapSettings>>;

#ifdef TEST_LIBCXX_NEW
template<typename TKey>
using set = momo::stdish::set<TKey>;
template<typename TKey>
using multiset = momo::stdish::multiset<TKey>;
#endif

LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_FAILURE
#define LIBCXX_TEST_PREFIX "libcxx_test_hash_multimap_" LIBCXX_TEST_PREFIX_TAIL
#include LIBCXX_HEADER(UnorderedMultiMapTests.h)
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_FAILURE

} // namespace libcxx_test_hash_multimap

} // namespace

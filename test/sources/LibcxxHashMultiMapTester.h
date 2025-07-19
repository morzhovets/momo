/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxHashMultiMapTester.h

\**********************************************************/

#pragma once

#include "LibcxxTester.h"

#include "../../include/momo/stdish/unordered_multimap.h"

#include "../../include/momo/stdish/set.h"

#ifndef LIBCXX_TEST_CLASS
# define LIBCXX_TEST_FAILURE
#endif

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_unord;

namespace libcxx_test_hash_multimap
{

#ifndef LIBCXX_TEST_CLASS

class LibcxxHashMultiMapSettings : public momo::HashMultiMapSettings
{
public:
#ifdef LIBCXX_TEST_FAILURE
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
#endif
};

#endif // LIBCXX_TEST_CLASS

namespace std
{
	using namespace ::std;

	template<typename TKey, typename TMapped,
		typename THasher = std::hash<TKey>,
		typename TEqualComparer = std::equal_to<TKey>,
		typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
#ifdef LIBCXX_TEST_CLASS
	using unordered_multimap = LIBCXX_TEST_CLASS<TKey, TMapped, THasher, TEqualComparer, TAllocator>;
#else
	using unordered_multimap = momo::stdish::unordered_multimap_adaptor<momo::HashMultiMap<TKey, TMapped,
		momo::HashTraitsStd<TKey, THasher, TEqualComparer, LIBCXX_TEST_BUCKET>,
		momo::MemManagerStd<TAllocator>,
		momo::HashMultiMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>>,
		LibcxxHashMultiMapSettings>>;
#endif

	template<typename TKey>
	using set = momo::stdish::set<TKey>;

	template<typename TKey>
	using multiset = momo::stdish::multiset<TKey>;
}

#define LIBCXX_TEST_PREFIX "libcxx_test_hash_multimap_" LIBCXX_TEST_PREFIX_TAIL
#include "libcxx/UnorderedMultiMapTests.h"
#undef LIBCXX_TEST_PREFIX

} // namespace libcxx_test_hash_multimap

} // namespace

#undef LIBCXX_TEST_FAILURE

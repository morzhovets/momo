/**********************************************************\

  tests/LibcxxHashSetTester.h

\**********************************************************/

#pragma once

#include "LibcxxTester.h"

#include "../../momo/stdish/unordered_set.h"

namespace
{

#ifdef MOMO_USE_TYPE_ALIASES

#define _LIBCPP_DEBUG 1
#define _LIBCPP_DEBUG_LEVEL 1

#define LIBCXX_TEST_PREFIX "libcxx_test_unordered_set_" LIBCXX_TEST_BUCKET
template<typename TKey,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<TKey>>
using unordered_set = momo::stdish::unordered_set<TKey, THashFunc, TEqualFunc, TAllocator,
	momo::HashSet<TKey, momo::HashTraitsStd<TKey, THashFunc, TEqualFunc>,
		momo::MemManagerStd<TAllocator>, momo::HashSetItemTraits<TKey>,
		momo::HashSetSettings<momo::CheckMode::exception>>>;
#include "LibcxxUnorderedSetTests.h"
#undef LIBCXX_TEST_PREFIX

#undef _LIBCPP_DEBUG
#undef _LIBCPP_DEBUG_LEVEL

#else

#define LIBCXX_TEST_PREFIX "libcxx_test_unordered_set_" LIBCXX_TEST_BUCKET
using momo::stdish::unordered_set;
#include "LibcxxUnorderedSetTests.h"
#undef LIBCXX_TEST_PREFIX

#endif

} // namespace

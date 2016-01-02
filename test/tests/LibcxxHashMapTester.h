/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxHashMapTester.h

\**********************************************************/

#pragma once

#include "LibcxxTester.h"

#include "../../momo/stdish/unordered_map.h"

namespace
{

#ifdef MOMO_USE_TYPE_ALIASES

#define _LIBCPP_DEBUG 1
#define _LIBCPP_DEBUG_LEVEL 1

#define LIBCXX_TEST_PREFIX "libcxx_test_unordered_map_" LIBCXX_TEST_BUCKET
struct LibcxxHashMapSettings : public momo::HashMapSettings
{
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};
template<typename TKey, typename TMapped,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using unordered_map = momo::stdish::unordered_map<TKey, TMapped, THashFunc, TEqualFunc, TAllocator,
	momo::HashMap<TKey, TMapped, momo::HashTraitsStd<TKey, THashFunc, TEqualFunc>,
		momo::MemManagerStd<TAllocator>, momo::HashMapKeyValueTraits<TKey, TMapped>,
		LibcxxHashMapSettings>>;
#include "LibcxxUnorderedMapTests.h"
#undef LIBCXX_TEST_PREFIX

#undef _LIBCPP_DEBUG
#undef _LIBCPP_DEBUG_LEVEL

#else

#define LIBCXX_TEST_PREFIX "libcxx_test_unordered_map_" LIBCXX_TEST_BUCKET
using momo::stdish::unordered_map;
#include "LibcxxUnorderedMapTests.h"
#undef LIBCXX_TEST_PREFIX

#endif

} // namespace

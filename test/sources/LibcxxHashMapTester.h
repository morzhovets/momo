/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxHashMapTester.h

\**********************************************************/

#pragma once

#include "LibcxxTester.h"

#include "../../include/momo/stdish/unordered_map.h"

namespace
{

#undef _LIBCPP_DEBUG
#undef _LIBCPP_DEBUG_LEVEL
#define _LIBCPP_DEBUG 1
#define _LIBCPP_DEBUG_LEVEL 1

#define LIBCXX_TEST_PREFIX "libcxx_test_hash_map_" LIBCXX_TEST_BUCKET_NAME
class LibcxxHashMapSettings : public momo::HashMapSettings
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};
template<typename TKey, typename TMapped,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using unordered_map = momo::stdish::unordered_map<TKey, TMapped, THashFunc, TEqualFunc, TAllocator,
	momo::HashMap<TKey, TMapped, momo::HashTraitsStd<TKey, THashFunc, TEqualFunc, LIBCXX_TEST_BUCKET>,
		momo::MemManagerStd<TAllocator>,
		momo::HashMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>>,
		LibcxxHashMapSettings>>;
#include "libcxx/UnorderedMapTests.h"
#undef LIBCXX_TEST_PREFIX

#undef _LIBCPP_DEBUG
#undef _LIBCPP_DEBUG_LEVEL

} // namespace

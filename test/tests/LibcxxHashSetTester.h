/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxHashSetTester.h

\**********************************************************/

#pragma once

#include "LibcxxTester.h"

#include "../../momo/stdish/unordered_set.h"

namespace
{

#undef _LIBCPP_DEBUG
#undef _LIBCPP_DEBUG_LEVEL
#define _LIBCPP_DEBUG 1
#define _LIBCPP_DEBUG_LEVEL 1

#define LIBCXX_TEST_PREFIX "libcxx_test_unordered_set_" LIBCXX_TEST_BUCKET_NAME
class LibcxxHashSetSettings : public momo::HashSetSettings
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};
template<typename TKey,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<TKey>>
using unordered_set = momo::stdish::unordered_set<TKey, THashFunc, TEqualFunc, TAllocator,
	momo::HashSet<TKey, momo::HashTraitsStd<TKey, THashFunc, TEqualFunc, LIBCXX_TEST_BUCKET>,
		momo::MemManagerStd<TAllocator>,
		momo::HashSetItemTraits<TKey, momo::MemManagerStd<TAllocator>>,
		LibcxxHashSetSettings>>;
#include "LibcxxUnorderedSetTests.h"
#undef LIBCXX_TEST_PREFIX

#undef _LIBCPP_DEBUG
#undef _LIBCPP_DEBUG_LEVEL

} // namespace

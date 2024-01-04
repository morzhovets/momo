/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxHashSetTester.h

\**********************************************************/

#pragma once

#include "LibcxxTester.h"

#include "../../include/momo/stdish/unordered_set.h"

namespace
{

class LibcxxHashSetSettings : public momo::HashSetSettings
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};

namespace libcxx_test_hash_set
{

LIBCXX_NAMESPACE_STD_BEGIN
template<typename TKey,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<TKey>>
using unordered_set = momo::stdish::unordered_set<TKey, THashFunc, TEqualFunc, TAllocator,
	momo::HashSet<TKey, momo::HashTraitsStd<TKey, THashFunc, TEqualFunc, LIBCXX_TEST_BUCKET>,
		momo::MemManagerStd<TAllocator>,
		momo::HashSetItemTraits<TKey, momo::MemManagerStd<TAllocator>>,
		LibcxxHashSetSettings>>;
LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_FAILURE
#define LIBCXX_TEST_PREFIX "libcxx_test_hash_set_" LIBCXX_TEST_BUCKET_NAME
#include LIBCXX_HEADER(UnorderedSetTests.h)
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_FAILURE

} // namespace libcxx_test_hash_set

} // namespace

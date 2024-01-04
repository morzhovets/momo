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

namespace
{

class LibcxxHashMultiMapSettings : public momo::HashMultiMapSettings
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};

namespace libcxx_test_hash_multimap
{

LIBCXX_NAMESPACE_STD_BEGIN
template<typename TKey, typename TMapped,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using unordered_multimap = momo::stdish::unordered_multimap<TKey, TMapped, THashFunc, TEqualFunc, TAllocator,
	momo::HashMultiMap<TKey, TMapped, momo::HashTraitsStd<TKey, THashFunc, TEqualFunc, LIBCXX_TEST_BUCKET>,
		momo::MemManagerStd<TAllocator>,
		momo::HashMultiMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>>,
		LibcxxHashMultiMapSettings>>;
LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_FAILURE
#define LIBCXX_TEST_PREFIX "libcxx_test_hash_multimap_" LIBCXX_TEST_BUCKET_NAME
#include LIBCXX_HEADER(UnorderedMultiMapTests.h)
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_FAILURE

} // namespace libcxx_test_hash_multimap

} // namespace

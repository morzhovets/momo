/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxHashMapTester.h

\**********************************************************/

#pragma once

#include "LibcxxTester.h"

#include "../../include/momo/stdish/unordered_map.h"

namespace
{

using namespace libcxx_from_range_unord;

class LibcxxHashMapSettings : public momo::HashMapSettings
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};

namespace libcxx_test_hash_map
{

namespace std
{
	using namespace ::std;

	template<typename TKey, typename TMapped,
		typename THashFunc = std::hash<TKey>,
		typename TEqualFunc = std::equal_to<TKey>,
		typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
	using unordered_map = momo::stdish::unordered_map<TKey, TMapped, THashFunc, TEqualFunc, TAllocator,
		momo::HashMap<TKey, TMapped, momo::HashTraitsStd<TKey, THashFunc, TEqualFunc, LIBCXX_TEST_BUCKET>,
			momo::MemManagerStd<TAllocator>,
			momo::HashMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>, false>,
			LibcxxHashMapSettings>>;
}

#define LIBCXX_TEST_FAILURE
#define LIBCXX_TEST_PREFIX "libcxx_test_hash_map_" LIBCXX_TEST_BUCKET_NAME
#include "libcxx/UnorderedMapTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_FAILURE

} // namespace libcxx_test_hash_map

#ifdef LIBCXX_TEST_DEFAULT_BUCKET

namespace libcxx_test_hash_map_vp
{

namespace std
{
	using namespace ::std;

	template<typename TKey, typename TMapped,
		typename THashFunc = std::hash<TKey>,
		typename TEqualFunc = std::equal_to<TKey>,
		typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
	using unordered_map = momo::stdish::unordered_map<TKey, TMapped, THashFunc, TEqualFunc, TAllocator,
		momo::HashMap<TKey, TMapped, momo::HashTraitsStd<TKey, THashFunc, TEqualFunc, LIBCXX_TEST_BUCKET>,
			momo::MemManagerStd<TAllocator>,
			momo::HashMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>, true>,
			LibcxxHashMapSettings>>;
}

#define LIBCXX_TEST_FAILURE
#define LIBCXX_TEST_PREFIX "libcxx_test_hash_map_vp_" LIBCXX_TEST_BUCKET_NAME
#include "libcxx/UnorderedMapTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_FAILURE

} // namespace libcxx_test_hash_map_vp

#endif

} // namespace

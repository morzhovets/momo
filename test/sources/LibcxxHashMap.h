/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxHashMap.h

\**********************************************************/

#pragma once

#include "Libcxx.h"

#include "../../include/momo/stdish/unordered_map.h"

#ifndef LIBCXX_TEST_CLASS
# define LIBCXX_TEST_FAILURE
#endif

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_unord;

namespace libcxx_test_hash_map
{

#ifndef LIBCXX_TEST_CLASS

template<typename TKey, typename THasher, typename TEqualComparer>
class LibcxxHashTraits
	: public momo::HashTraitsStd<TKey, THasher, TEqualComparer, LIBCXX_TEST_HASH_BUCKET>
{
private:
	typedef momo::HashTraitsStd<TKey, THasher, TEqualComparer, LIBCXX_TEST_HASH_BUCKET> HashTraitsStd;

public:
#ifdef LIBCXX_TEST_HINT_ITERATORS
	static const bool useHintIterators = true;
#endif

public:
	using HashTraitsStd::HashTraitsStd;
};

template<typename TKey, typename TMapped, typename TAllocator>
class LibcxxHashMapKeyValueTraits
#ifdef LIBCXX_TEST_MAP_VALUE_PTR
	: public momo::HashMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>, true>
#else
	: public momo::HashMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>, false>
#endif
{
public:
#ifdef LIBCXX_TEST_SAFE_MAP_BRACKETS
	static const bool useSafeValueReference = true;
#endif
};

class LibcxxHashMapSettings : public momo::HashMapSettings
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
	using unordered_map = LIBCXX_TEST_CLASS<TKey, TMapped, THasher, TEqualComparer, TAllocator>;
#else
	using unordered_map = momo::stdish::unordered_map_adaptor<momo::HashMap<TKey, TMapped,
		LibcxxHashTraits<TKey, THasher, TEqualComparer>, momo::MemManagerStd<TAllocator>,
		LibcxxHashMapKeyValueTraits<TKey, TMapped, TAllocator>,
		LibcxxHashMapSettings>>;
#endif
}

#define LIBCXX_TEST_PREFIX "libcxx_test_hash_map" LIBCXX_TEST_PREFIX_TAIL
#include "libcxx/UnorderedMapTests.h"
#undef LIBCXX_TEST_PREFIX

} // namespace libcxx_test_hash_map

} // namespace

#undef LIBCXX_TEST_FAILURE

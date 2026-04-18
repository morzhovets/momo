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

#if !defined(LIBCXX_TEST_CLASS) && !defined(TEST_HAS_NO_EXCEPTIONS)
# define LIBCXX_TEST_FAILURE
#endif

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_unord;

namespace libcxx_hash_map
{

#ifndef LIBCXX_TEST_CLASS

template<typename TKey, typename THasher, typename TEqualComparer>
class LibcxxHashTraits : public momo::HashTraitsStd<TKey, THasher, TEqualComparer, LIBCXX_TEST_HASH_BUCKET>
{
private:
	typedef momo::HashTraitsStd<TKey, THasher, TEqualComparer, LIBCXX_TEST_HASH_BUCKET> HashTraitsBase;

public:
#ifdef LIBCXX_TEST_HINT_ITERATORS
	static const bool useHintIterators = true;
#endif

public:
	using HashTraitsBase::HashTraitsBase;
};

template<typename TKey, typename TMapped, typename TAllocator>
class LibcxxHashMapKeyValueTraits
	: public momo::HashMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>>
{
public:
#ifdef LIBCXX_TEST_SAFE_MAP_BRACKETS
	static const bool useSafeValueReference = true;
#endif
};

class LibcxxHashMapSettings : public momo::HashMapSettings
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION;
};

#endif // LIBCXX_TEST_CLASS

LIBCXX_NAMESPACE_STD_BEGIN
template<typename TKey, typename TMapped,
	typename THasher = std::hash<TKey>,
	typename TEqualComparer = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
#ifdef LIBCXX_TEST_CLASS
using unordered_map = LIBCXX_TEST_CLASS<TKey, TMapped, THasher, TEqualComparer, TAllocator>;
#else
using unordered_map = momo::stdish::unordered_map_adaptor<momo::HashMapCore<
	LibcxxHashMapKeyValueTraits<TKey, TMapped, TAllocator>,
	LibcxxHashTraits<TKey, THasher, TEqualComparer>, LibcxxHashMapSettings>>;
#endif
LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_PREFIX "hash_map" LIBCXX_TEST_PREFIX_TAIL
#include LIBCXX_HEADER(UnorderedMapTests.h)
#undef LIBCXX_TEST_PREFIX

} // namespace libcxx_hash_map

} // namespace

#undef LIBCXX_TEST_FAILURE

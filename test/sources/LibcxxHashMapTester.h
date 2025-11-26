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

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_unord;

namespace libcxx_hash_map
{

class LibcxxHashMapSettings : public momo::HashMapSettings
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};

template<typename TKey, typename THasher, typename TEqualComparer>
class LibcxxHashTraits : public momo::HashTraitsStd<TKey, THasher, TEqualComparer, LIBCXX_TEST_BUCKET>
{
private:
	typedef momo::HashTraitsStd<TKey, THasher, TEqualComparer, LIBCXX_TEST_BUCKET> HashTraitsBase;

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

LIBCXX_NAMESPACE_STD_BEGIN
template<typename TKey, typename TMapped,
	typename THasher = std::hash<TKey>,
	typename TEqualComparer = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using unordered_map = momo::stdish::unordered_map<TKey, TMapped, THasher, TEqualComparer, TAllocator,
	momo::HashMapCore<LibcxxHashMapKeyValueTraits<TKey, TMapped, TAllocator>,
		LibcxxHashTraits<TKey, THasher, TEqualComparer>, LibcxxHashMapSettings>>;
LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_FAILURE
#define LIBCXX_TEST_PREFIX "libcxx_hash_map_" LIBCXX_TEST_PREFIX_TAIL
#include LIBCXX_HEADER(UnorderedMapTests.h)
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_FAILURE

} // namespace libcxx_hash_map

} // namespace

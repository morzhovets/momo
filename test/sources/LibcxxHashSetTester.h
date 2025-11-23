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

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_unord;

namespace libcxx_test_hash_set
{

class LibcxxHashSetSettings : public momo::HashSetSettings
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

LIBCXX_NAMESPACE_STD_BEGIN
template<typename TKey,
	typename THasher = std::hash<TKey>,
	typename TEqualComparer = std::equal_to<TKey>,
	typename TAllocator = std::allocator<TKey>>
using unordered_set = momo::stdish::unordered_set<TKey, THasher, TEqualComparer, TAllocator,
	momo::HashSetCore<momo::HashSetItemTraits<TKey, momo::MemManagerStd<TAllocator>>,
		LibcxxHashTraits<TKey, THasher, TEqualComparer>, LibcxxHashSetSettings>>;
LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_FAILURE
#define LIBCXX_TEST_PREFIX "libcxx_test_hash_set_" LIBCXX_TEST_PREFIX_TAIL
#include LIBCXX_HEADER(UnorderedSetTests.h)
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_FAILURE

} // namespace libcxx_test_hash_set

} // namespace

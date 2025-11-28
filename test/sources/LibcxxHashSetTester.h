/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxHashSetTester.h

\**********************************************************/

#pragma once

#include "LibcxxTester.h"

#include "../../include/momo/stdish/unordered_set.h"

#ifndef LIBCXX_TEST_CLASS
# define LIBCXX_TEST_FAILURE
#endif

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_unord;

namespace libcxx_hash_set
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

class LibcxxHashSetSettings : public momo::HashSetSettings
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

	template<typename TKey,
		typename THasher = std::hash<TKey>,
		typename TEqualComparer = std::equal_to<TKey>,
		typename TAllocator = std::allocator<TKey>>
#ifdef LIBCXX_TEST_CLASS
	using unordered_set = LIBCXX_TEST_CLASS<TKey, THasher, TEqualComparer, TAllocator>;
#else
	using unordered_set = momo::stdish::unordered_set_adaptor<momo::HashSetCore<
		momo::HashSetItemTraits<TKey, momo::MemManagerStd<TAllocator>>,
		LibcxxHashTraits<TKey, THasher, TEqualComparer>, LibcxxHashSetSettings>>;
#endif
}

#define LIBCXX_TEST_PREFIX "libcxx_hash_set" LIBCXX_TEST_PREFIX_TAIL
#include "libcxx/UnorderedSetTests.h"
#undef LIBCXX_TEST_PREFIX

} // namespace libcxx_hash_set

} // namespace

#undef LIBCXX_TEST_FAILURE

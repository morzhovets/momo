/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxTreeMap.h

\**********************************************************/

#pragma once

#include "Libcxx.h"

#include "../../include/momo/stdish/map.h"

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_assoc;

namespace libcxx_tree_map
{

#ifndef LIBCXX_TEST_CLASS

template<typename TKey, typename TLessComparer>
using LibcxxTreeTraits = momo::TreeTraitsStd<TKey, TLessComparer, false, LIBCXX_TEST_TREE_NODE>;

template<typename TKey, typename TMapped, typename TAllocator>
class LibcxxTreeMapKeyValueTraits
#ifdef LIBCXX_TEST_MAP_VALUE_PTR
	: public momo::TreeMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>, true>
#else
	: public momo::TreeMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>, false>
#endif
{
public:
#ifdef LIBCXX_TEST_SAFE_MAP_BRACKETS
	static const bool useSafeValueReference = true;
#endif
};

#endif // LIBCXX_TEST_CLASS

namespace std
{
	using namespace ::std;

	template<typename TKey, typename TMapped,
		typename TLessComparer = std::less<TKey>,
		typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
#ifdef LIBCXX_TEST_CLASS
	using map = LIBCXX_TEST_CLASS<TKey, TMapped, TLessComparer, TAllocator>;
#else
	using map = momo::stdish::map_adaptor<momo::TreeMap<TKey, TMapped,
		LibcxxTreeTraits<TKey, TLessComparer>, momo::MemManagerStd<TAllocator>,
		LibcxxTreeMapKeyValueTraits<TKey, TMapped, TAllocator>, momo::TreeMapSettings>>;
#endif
}

#define LIBCXX_TEST_PREFIX "libcxx_tree_map" LIBCXX_TEST_PREFIX_TAIL
#include "libcxx/MapTests.h"
#undef LIBCXX_TEST_PREFIX

} // namespace libcxx_tree_map

} // namespace

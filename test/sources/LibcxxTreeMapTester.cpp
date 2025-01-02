/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxTreeMapTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_TREE_MAP

#include "LibcxxTester.h"

#include "../../include/momo/stdish/map.h"

#include "../../include/momo/stdish/vector.h"

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_assoc;

namespace libcxx_test_tree_map
{

namespace std
{
	using namespace ::std;

	template<typename TKey, typename TMapped,
		typename TLessComparer = std::less<TKey>,
		typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
	using map = momo::stdish::map<TKey, TMapped, TLessComparer, TAllocator,
		momo::TreeMap<TKey, TMapped, momo::TreeTraitsStd<TKey, TLessComparer, false,
			momo::TreeNode<>>,
			momo::MemManagerStd<TAllocator>,
			momo::TreeMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>, false>,
			momo::TreeMapSettings>>;

	template<typename TValue>
	using vector = momo::stdish::vector<TValue>;
}

#define LIBCXX_TEST_PREFIX "libcxx_test_tree_map"
#include "libcxx/MapTests.h"
#undef LIBCXX_TEST_PREFIX

} // namespace libcxx_test_tree_map

} // namespace

#endif // TEST_LIBCXX_TREE_MAP

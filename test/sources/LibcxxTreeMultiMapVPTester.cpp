/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxTreeMultiMapVPTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_TREE_MAP

#include "LibcxxTester.h"

#include "../../include/momo/stdish/map.h"

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_assoc;

namespace libcxx_test_tree_multimap_vp
{

namespace std
{
	using namespace ::std;

	template<typename TKey, typename TMapped,
		typename TLessComparer = std::less<TKey>,
		typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
	using multimap = momo::stdish::multimap<TKey, TMapped, TLessComparer, TAllocator,
		momo::TreeMap<TKey, TMapped, momo::TreeTraitsStd<TKey, TLessComparer, true,
			momo::TreeNode<4, 2, momo::MemPoolParams<1>, true>>,
			momo::MemManagerStd<TAllocator>,
			momo::TreeMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>, true>,
			momo::TreeMapSettings>>;
}

#define LIBCXX_TEST_PREFIX "libcxx_test_tree_multimap_vp"
#include "libcxx/MultiMapTests.h"
#undef LIBCXX_TEST_PREFIX

} // namespace libcxx_test_tree_multimap_vp

} // namespace

#endif // TEST_LIBCXX_TREE_MAP

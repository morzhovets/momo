/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxTreeMapTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_TREE_MAP

#include "LibcxxTester.h"

#include "../../include/momo/stdish/map.h"

#define LIBCXX_TEST_PREFIX "libcxx_test_tree_map"
namespace libcxx_test_tree_map
{
template<typename TKey, typename TMapped,
	typename TLessFunc = std::less<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using map = momo::stdish::map<TKey, TMapped, TLessFunc, TAllocator,
	momo::TreeMap<TKey, TMapped, momo::TreeTraitsStd<TKey, TLessFunc, false,
		momo::TreeNode<>>,
		momo::MemManagerStd<TAllocator>,
		momo::TreeMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>, false>,
		momo::TreeMapSettings>>;
#include "LibcxxMapTests.h"
}
#undef LIBCXX_TEST_PREFIX

#define LIBCXX_TEST_PREFIX "libcxx_test_tree_map_vp"
namespace libcxx_test_tree_map_vp
{
template<typename TKey, typename TMapped,
	typename TLessFunc = std::less<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using map = momo::stdish::map<TKey, TMapped, TLessFunc, TAllocator,
	momo::TreeMap<TKey, TMapped, momo::TreeTraitsStd<TKey, TLessFunc, false,
		momo::TreeNode<32, 4, momo::MemPoolParams<>, false>>,
		momo::MemManagerStd<TAllocator>,
		momo::TreeMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>, true>,
		momo::TreeMapSettings>>;
#include "LibcxxMapTests.h"
}
#undef LIBCXX_TEST_PREFIX

#endif // TEST_LIBCXX_TREE_MAP

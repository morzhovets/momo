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

namespace libcxx_test_tree_map
{

namespace std
{
	using namespace ::std;

	template<typename TKey, typename TMapped,
		typename TLessFunc = std::less<TKey>,
		typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
	using map = momo::stdish::map<TKey, TMapped, TLessFunc, TAllocator,
		momo::TreeMap<TKey, TMapped, momo::TreeTraitsStd<TKey, TLessFunc, false,
			momo::TreeNode<>>,
			momo::MemManagerStd<TAllocator>,
			momo::TreeMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>, false>,
			momo::TreeMapSettings>>;
}

using std::map;

#define LIBCXX_TEST_PREFIX "libcxx_test_tree_map"
#include "LibcxxMapTests.h"
#undef LIBCXX_TEST_PREFIX

}

namespace libcxx_test_tree_map_vp
{

namespace std
{
	using namespace ::std;

	template<typename TKey, typename TMapped,
		typename TLessFunc = std::less<TKey>,
		typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
	using map = momo::stdish::map<TKey, TMapped, TLessFunc, TAllocator,
		momo::TreeMap<TKey, TMapped, momo::TreeTraitsStd<TKey, TLessFunc, false,
			momo::TreeNode<32, 4, momo::MemPoolParams<>, false>>,
			momo::MemManagerStd<TAllocator>,
			momo::TreeMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>, true>,
			momo::TreeMapSettings>>;
}

using std::map;

#define LIBCXX_TEST_PREFIX "libcxx_test_tree_map_vp"
#include "LibcxxMapTests.h"
#undef LIBCXX_TEST_PREFIX

}

#endif // TEST_LIBCXX_TREE_MAP

/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxTreeMultiMapTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_TREE_MAP

#include "LibcxxTester.h"

#include "../../include/momo/stdish/map.h"

namespace libcxx_test_tree_multimap
{

namespace std
{
	using namespace ::std;

	template<typename TKey, typename TMapped,
		typename TLessFunc = std::less<TKey>,
		typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
	using multimap = momo::stdish::multimap<TKey, TMapped, TLessFunc, TAllocator,
		momo::TreeMap<TKey, TMapped, momo::TreeTraitsStd<TKey, TLessFunc, true,
			momo::TreeNode<4, 2, momo::MemPoolParams<1>, false>>,
			momo::MemManagerStd<TAllocator>,
			momo::TreeMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>, false>,
			momo::TreeMapSettings>>;
}

#define LIBCXX_TEST_PREFIX "libcxx_test_tree_multimap"
#include "LibcxxMultiMapTests.h"
#undef LIBCXX_TEST_PREFIX
}

namespace libcxx_test_tree_multimap_vp
{

namespace std
{
	using namespace ::std;

	template<typename TKey, typename TMapped,
		typename TLessFunc = std::less<TKey>,
		typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
	using multimap = momo::stdish::multimap<TKey, TMapped, TLessFunc, TAllocator,
		momo::TreeMap<TKey, TMapped, momo::TreeTraitsStd<TKey, TLessFunc, true,
			momo::TreeNode<4, 2, momo::MemPoolParams<1>, true>>,
			momo::MemManagerStd<TAllocator>,
			momo::TreeMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>, true>,
			momo::TreeMapSettings>>;
}

#define LIBCXX_TEST_PREFIX "libcxx_test_tree_multimap_vp"
#include "LibcxxMultiMapTests.h"
#undef LIBCXX_TEST_PREFIX
}

#endif // TEST_LIBCXX_TREE_MAP

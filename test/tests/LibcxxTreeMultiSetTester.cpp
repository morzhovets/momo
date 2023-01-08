/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  tests/LibcxxTreeMultiSetTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_TREE_SET

#include "LibcxxTester.h"

#include "../../momo/stdish/set.h"

namespace
{

#define LIBCXX_TEST_PREFIX "libcxx_test_tree_multiset"
template<typename TKey,
	typename TLessFunc = std::less<TKey>,
	typename TAllocator = std::allocator<TKey>>
using multiset = momo::stdish::multiset<TKey, TLessFunc, TAllocator,
	momo::TreeSet<TKey, momo::TreeTraitsStd<TKey, TLessFunc, true,
		momo::TreeNode<4, 2, momo::MemPoolParams<>, true>>,
		momo::MemManagerStd<TAllocator>,
		momo::TreeSetItemTraits<TKey, momo::MemManagerStd<TAllocator>>,
		momo::TreeSetSettings>>;
#include "LibcxxMultiSetTests.h"
#undef LIBCXX_TEST_PREFIX

} // namespace

#endif // TEST_LIBCXX_TREE_SET

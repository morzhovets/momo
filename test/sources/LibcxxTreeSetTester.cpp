/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxTreeSetTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_TREE_SET

#include "LibcxxTester.h"

#include "../../include/momo/stdish/set.h"

namespace
{

#define LIBCXX_TEST_PREFIX "libcxx_test_tree_set"
template<typename TKey,
	typename TLessFunc = std::less<TKey>,
	typename TAllocator = std::allocator<TKey>>
using set = momo::stdish::set<TKey, TLessFunc, TAllocator,
	momo::TreeSet<TKey, momo::TreeTraitsStd<TKey, TLessFunc, false,
		momo::TreeNode<32, 4, momo::MemPoolParams<1>, true>>,
		momo::MemManagerStd<TAllocator>,
		momo::TreeSetItemTraits<TKey, momo::MemManagerStd<TAllocator>>,
		momo::TreeSetSettings>>;
#include "LibcxxSetTests.h"
#undef LIBCXX_TEST_PREFIX

} // namespace

#endif // TEST_LIBCXX_TREE_SET

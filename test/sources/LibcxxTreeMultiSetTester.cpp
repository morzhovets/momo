/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxTreeMultiSetTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_TREE_SET

#include "LibcxxTester.h"

#include "../../include/momo/stdish/set.h"

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_assoc;

namespace libcxx_test_tree_multiset
{

LIBCXX_NAMESPACE_STD_BEGIN
template<typename TKey,
	typename TLessComparer = std::less<TKey>,
	typename TAllocator = std::allocator<TKey>>
using multiset = momo::stdish::multiset<TKey, TLessComparer, TAllocator,
	momo::TreeSet<TKey, momo::TreeTraitsStd<TKey, TLessComparer, true,
		momo::TreeNode<4, 2, momo::MemPoolParams<1>, true>>,
		momo::MemManagerStd<TAllocator>,
		momo::TreeSetItemTraits<TKey, momo::MemManagerStd<TAllocator>>,
		momo::TreeSetSettings>>;
LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_PREFIX "libcxx_test_tree_multiset"
#include LIBCXX_HEADER(MultiSetTests.h)
#undef LIBCXX_TEST_PREFIX

} // namespace libcxx_test_tree_multiset

} // namespace

#endif // TEST_LIBCXX_TREE_SET

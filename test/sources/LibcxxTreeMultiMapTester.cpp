/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxTreeMultiMapTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_TREE_MAP

#include "LibcxxTester.h"

#include "../../include/momo/stdish/map.h"

namespace libcxx_test_tree_multimap
{

LIBCXX_NAMESPACE_STD_BEGIN
template<typename TKey, typename TMapped,
	typename TLessFunc = std::less<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using multimap = momo::stdish::multimap<TKey, TMapped, TLessFunc, TAllocator,
	momo::TreeMap<TKey, TMapped, momo::TreeTraitsStd<TKey, TLessFunc, true,
		momo::TreeNode<32, 4, momo::MemPoolParams<1>, false>>,
		momo::MemManagerStd<TAllocator>,
		momo::TreeMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>>,
		momo::TreeMapSettings>>;
LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_PREFIX "libcxx_test_tree_multimap"
#include LIBCXX_HEADER(MultiMapTests.h)
#undef LIBCXX_TEST_PREFIX

} // namespace libcxx_test_tree_multimap

#endif // TEST_LIBCXX_TREE_MAP

/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxTreeMapTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_TREE_MAP

#include "LibcxxTester.h"

#include "../../include/momo/stdish/map.h"

#ifdef TEST_LIBCXX_NEW
#include "../../include/momo/stdish/vector.h"
#endif

namespace libcxx_test_tree_map
{

LIBCXX_NAMESPACE_STD_BEGIN
template<typename TKey, typename TMapped,
	typename TLessFunc = std::less<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using map = momo::stdish::map<TKey, TMapped, TLessFunc, TAllocator,
	momo::TreeMap<TKey, TMapped, momo::TreeTraitsStd<TKey, TLessFunc, false,
		momo::TreeNode<>>,
		momo::MemManagerStd<TAllocator>,
		momo::TreeMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>>,
		momo::TreeMapSettings>>;
LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_PREFIX "libcxx_test_tree_map"
#include LIBCXX_HEADER(MapTests.h)
#undef LIBCXX_TEST_PREFIX

} // namespace libcxx_test_tree_map

#endif // TEST_LIBCXX_TREE_MAP

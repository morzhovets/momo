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

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_assoc;

namespace libcxx_tree_multimap
{

LIBCXX_NAMESPACE_STD_BEGIN
template<typename TKey, typename TMapped,
	typename TLessComparer = std::less<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using multimap = momo::stdish::multimap<TKey, TMapped, TLessComparer, TAllocator,
	momo::TreeMapCore<momo::TreeMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>>,
		momo::TreeTraitsStd<TKey, TLessComparer, true, momo::TreeNode<4, 2, momo::MemPoolParams<>, false>>,
		momo::TreeMapSettings>>;
LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_PREFIX "tree_multimap"
#include LIBCXX_HEADER(MultiMapTests.h)
#undef LIBCXX_TEST_PREFIX

} // namespace libcxx_tree_multimap

} // namespace

#endif // TEST_LIBCXX_TREE_MAP

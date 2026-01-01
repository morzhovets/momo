/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxTreeMapTester_SB.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_TREE_MAP

#include "LibcxxTester.h"

#include "../../include/momo/stdish/map.h"

#if TEST_LIBCXX_VERSION >= 20
# include "../../include/momo/stdish/vector.h"
#endif

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_assoc;

namespace libcxx_tree_map
{

template<typename TKey, typename TMapped, typename TAllocator>
class LibcxxTreeMapKeyValueTraits
	: public momo::TreeMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>>
{
public:
	static const bool useSafeValueReference = true;
};

LIBCXX_NAMESPACE_STD_BEGIN

template<typename TKey, typename TMapped,
	typename TLessComparer = std::less<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using map = momo::stdish::map<TKey, TMapped, TLessComparer, TAllocator,
	momo::TreeMapCore<LibcxxTreeMapKeyValueTraits<TKey, TMapped, TAllocator>,
		momo::TreeTraitsStd<TKey, TLessComparer, false, momo::TreeNode<32, 4, momo::MemPoolParams<1>, false>>,
		momo::TreeMapSettings>>;

#if TEST_LIBCXX_VERSION >= 20
template<typename TValue>
using vector = momo::stdish::vector<TValue>;
#endif

LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_SAFE_MAP_BRACKETS
#define LIBCXX_TEST_PREFIX "tree_map_sb"
#include LIBCXX_HEADER(MapTests.h)
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_SAFE_MAP_BRACKETS

} // namespace libcxx_tree_map

} // namespace

#endif // TEST_LIBCXX_TREE_MAP

/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxTreeMapTester_SB.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_TREE_MAP

#include "LibcxxTester.h"

#include "../../include/momo/stdish/map.h"

#include "../../include/momo/stdish/vector.h"

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_assoc;

namespace libcxx_test_tree_map
{

template<typename TKey, typename TMapped, typename TAllocator>
class LibcxxTreeMapKeyValueTraits
	: public momo::TreeMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>, false>
{
public:
	static const bool useSafeValueReference = true;
};

namespace std
{
	using namespace ::std;

	template<typename TKey, typename TMapped,
		typename TLessComparer = std::less<TKey>,
		typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
	using map = momo::stdish::map_adaptor<momo::TreeMap<TKey, TMapped,
		momo::TreeTraitsStd<TKey, TLessComparer, false,
			momo::TreeNode<4, 2, momo::MemPoolParams<1>, false>>,
		momo::MemManagerStd<TAllocator>,
		LibcxxTreeMapKeyValueTraits<TKey, TMapped, TAllocator>,
		momo::TreeMapSettings>>;

	template<typename TValue>
	using vector = momo::stdish::vector<TValue>;
}

#define LIBCXX_TEST_SAFE_MAP_BRACKETS
#define LIBCXX_TEST_PREFIX "libcxx_test_tree_map_sb"
#include "libcxx/MapTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_SAFE_MAP_BRACKETS

} // namespace libcxx_test_tree_map

} // namespace

#endif // TEST_LIBCXX_TREE_MAP

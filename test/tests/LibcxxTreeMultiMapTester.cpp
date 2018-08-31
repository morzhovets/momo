/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxTreeMultiMapTester.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_TREE_MAP

#undef NDEBUG

#include "../../momo/Utility.h"

#include "LibcxxTester.h"

#include "../../momo/stdish/map.h"

namespace
{

#define LIBCXX_TEST_PREFIX "libcxx_test_multimap"
template<typename TKey, typename TMapped,
	typename TLessFunc = std::less<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using multimap = momo::stdish::multimap<TKey, TMapped, TLessFunc, TAllocator,
	momo::TreeMap<TKey, TMapped, momo::TreeTraitsStd<TKey, TLessFunc, true,
		momo::TreeNode<32, 4, momo::MemPoolParams<1>, false>>,
		momo::MemManagerStd<TAllocator>,
		momo::TreeMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>>,
		momo::TreeMapSettings>>;
#include "LibcxxMultiMapTests.h"
#undef LIBCXX_TEST_PREFIX

} // namespace

#endif // TEST_LIBCXX_TREE_MAP

/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxTreeMapTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_TREE_MAP

#include "LibcxxTester.h"

#include "../../momo/stdish/map.h"

namespace
{

#define LIBCXX_TEST_PREFIX "libcxx_test_map"
template<typename TKey, typename TMapped,
	typename TLessFunc = std::less<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using map = momo::stdish::map<TKey, TMapped, TLessFunc, TAllocator,
	momo::TreeMap<TKey, TMapped, momo::TreeTraitsStd<TKey, TLessFunc, false,
		momo::TreeNode<32, 4, momo::MemPoolParams<>, true>>,
		momo::MemManagerStd<TAllocator>,
		momo::TreeMapKeyValueTraits<TKey, TMapped, momo::MemManagerStd<TAllocator>>,
		momo::TreeMapSettings>>;
#include "LibcxxMapTests.h"
#undef LIBCXX_TEST_PREFIX

} // namespace

#endif // TEST_LIBCXX_TREE_MAP

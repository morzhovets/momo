/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
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

namespace libcxx_tree_multiset
{

namespace std
{
	using namespace ::std;

	template<typename TKey,
		typename TLessComparer = std::less<TKey>,
		typename TAllocator = std::allocator<TKey>>
	using multiset = momo::stdish::multiset<TKey, TLessComparer, TAllocator>;
}

#define LIBCXX_TEST_CLASS momo::stdish::multiset
#define LIBCXX_TEST_PREFIX "libcxx_tree_multiset"
#include "libcxx/MultiSetTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_CLASS

} // namespace libcxx_tree_multiset

} // namespace

#endif // TEST_LIBCXX_TREE_SET

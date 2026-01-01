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

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_assoc;

namespace libcxx_tree_set
{

namespace std
{
	using namespace ::std;

	template<typename TKey,
		typename TLessComparer = std::less<TKey>,
		typename TAllocator = std::allocator<TKey>>
	using set = momo::stdish::set<TKey, TLessComparer, TAllocator>;
}

#define LIBCXX_TEST_CLASS momo::stdish::set
#define LIBCXX_TEST_PREFIX "tree_set"
#include "libcxx20/SetTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_CLASS

} // namespace libcxx_tree_set

} // namespace

#endif // TEST_LIBCXX_TREE_SET

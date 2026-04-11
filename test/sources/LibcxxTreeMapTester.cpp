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

#if TEST_LIBCXX_VERSION >= 20
# include "../../include/momo/stdish/vector.h"
#endif

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_assoc;

namespace libcxx_tree_map
{

LIBCXX_NAMESPACE_STD_BEGIN

using momo::stdish::map;

#if TEST_LIBCXX_VERSION >= 20
using momo::stdish::vector;
#endif

LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_CLASS momo::stdish::map
#define LIBCXX_TEST_PREFIX "tree_map"
#include LIBCXX_HEADER(MapTests.h)
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_CLASS

} // namespace libcxx_tree_map

} // namespace

#endif // TEST_LIBCXX_TREE_MAP

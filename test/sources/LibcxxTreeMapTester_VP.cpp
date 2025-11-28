/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxTreeMapTester_VP.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_TREE_MAP

#define LIBCXX_TEST_TREE_NODE momo::TreeNode<4, 2, momo::MemPoolParams<>, true>
#define LIBCXX_TEST_PREFIX_TAIL "_vp"
#define LIBCXX_TEST_MAP_VALUE_PTR

#include "LibcxxTreeMapTester.h"

#endif // TEST_LIBCXX_TREE_MAP

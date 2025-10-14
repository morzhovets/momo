/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxTreeMap_SB.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_TREE_MAP

#define LIBCXX_TEST_TREE_NODE momo::TreeNode<4, 2, momo::MemPoolParams<1>, false>
#define LIBCXX_TEST_PREFIX_TAIL "_sb"
#define LIBCXX_TEST_SAFE_MAP_BRACKETS

#include "LibcxxTreeMap.h"

#endif // TEST_LIBCXX_TREE_MAP

/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxHashMapTester_LimP4_SB.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_MAP

#include "../../include/momo/details/HashBucketLimP4.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLimP4<3>
#define LIBCXX_TEST_PREFIX_TAIL "limp4_sb"
#define LIBCXX_TEST_SAFE_MAP_BRACKETS

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

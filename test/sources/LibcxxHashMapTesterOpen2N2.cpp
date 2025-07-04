/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxHashMapTesterOpen2N2.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_MAP

#include "../../include/momo/details/HashBucketOpen2N2.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketOpen2N2<>
#define LIBCXX_TEST_PREFIX_TAIL "open2n2"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

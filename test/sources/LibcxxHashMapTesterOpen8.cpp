/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxHashMapTesterOpen8.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_MAP

#include "../../include/momo/details/HashBucketOpen8.h"

#define LIBCXX_TEST_CLASS momo::stdish::unordered_map_open
#define LIBCXX_TEST_PREFIX_TAIL "open8"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

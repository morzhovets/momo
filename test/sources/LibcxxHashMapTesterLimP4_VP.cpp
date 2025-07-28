/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxHashMapTesterLimP4_VP.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_MAP

#include "../../include/momo/details/HashBucketLimP4.h"

#define LIBCXX_TEST_HASH_BUCKET momo::HashBucketLimP4<>
#define LIBCXX_TEST_PREFIX_TAIL "_limp4_vp"
#define LIBCXX_TEST_MAP_VALUE_PTR

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

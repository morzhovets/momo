/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxHashMapTester_LimP4_HI.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_MAP

#include "../../include/momo/details/HashBucketLimP4.h"

#define LIBCXX_TEST_HASH_BUCKET momo::HashBucketLimP4<1>
#define LIBCXX_TEST_PREFIX_TAIL "_limp4_hi"
#define LIBCXX_TEST_HINT_ITERATORS

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxHashMapLimP1.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_MAP
#ifdef TEST_OLD_HASH_BUCKETS

#include "../../include/momo/details/HashBucketLimP1.h"

#define LIBCXX_TEST_HASH_BUCKET momo::HashBucketLimP1<>
#define LIBCXX_TEST_PREFIX_TAIL "_limp1"

#include "LibcxxHashMap.h"

#endif // TEST_OLD_HASH_BUCKETS
#endif // TEST_LIBCXX_HASH_MAP

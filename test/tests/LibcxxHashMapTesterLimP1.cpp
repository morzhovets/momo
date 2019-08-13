/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxHashMapTesterLimP1.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP
#ifdef TEST_OLD_HASH_BUCKETS

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/HashBucketLimP1.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLimP1<>
#define LIBCXX_TEST_BUCKET_NAME "limp1"

#include "LibcxxHashMapTester.h"

#endif // TEST_OLD_HASH_BUCKETS
#endif // TEST_LIBCXX_HASH_MAP

/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxHashMapTesterOne.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_MAP
#ifdef TEST_OLD_HASH_BUCKETS

#include "../../include/momo/details/HashBucketOne.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketOne<>
#define LIBCXX_TEST_PREFIX_TAIL "one"

#include "LibcxxHashMapTester.h"

#endif // TEST_OLD_HASH_BUCKETS
#endif // TEST_LIBCXX_HASH_MAP

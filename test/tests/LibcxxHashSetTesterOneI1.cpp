/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxHashSetTesterOneI1.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_SET
#ifdef TEST_OLD_HASH_BUCKETS

#include "../../momo/details/HashBucketOneI1.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketOneI1
#define LIBCXX_TEST_BUCKET_NAME "onei1"

#include "LibcxxHashSetTester.h"

#endif // TEST_OLD_HASH_BUCKETS
#endif // TEST_LIBCXX_HASH_SET

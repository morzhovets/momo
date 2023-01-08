/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  tests/LibcxxHashSetTesterLimP.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_SET
#ifdef TEST_OLD_HASH_BUCKETS

#include "../../momo/details/HashBucketLimP.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLimP<sizeof(void*), momo::MemPoolParams<>, false>
#define LIBCXX_TEST_BUCKET_NAME "limp"

#include "LibcxxHashSetTester.h"

#endif // TEST_OLD_HASH_BUCKETS
#endif // TEST_LIBCXX_HASH_SET

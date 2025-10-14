/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxHashSetLimP.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_SET
#ifdef TEST_OLD_HASH_BUCKETS

#include "../../include/momo/details/HashBucketLimP.h"

#define LIBCXX_TEST_HASH_BUCKET momo::HashBucketLimP<sizeof(void*), momo::MemPoolParams<>, false>
#define LIBCXX_TEST_PREFIX_TAIL "_limp"

#include "LibcxxHashSet.h"

#endif // TEST_OLD_HASH_BUCKETS
#endif // TEST_LIBCXX_HASH_SET

/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxHashSetTesterLimP1.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/HashBucketLimP1.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLimP1<1, momo::MemPoolParams<1>, 1>
#define LIBCXX_TEST_BUCKET_NAME "limp1"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

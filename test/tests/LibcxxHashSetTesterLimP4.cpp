/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxHashSetTesterLimP4.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/HashBucketLimP4.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLimP4<1, momo::MemPoolParams<1>>
#define LIBCXX_TEST_BUCKET_NAME "limp4"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

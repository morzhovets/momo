/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxHashSetTesterLimP.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET

#undef NDEBUG

#include "../../momo/Utility.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLimP<1, momo::MemPoolParams<1>, true>
#define LIBCXX_TEST_BUCKET_NAME "limp"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

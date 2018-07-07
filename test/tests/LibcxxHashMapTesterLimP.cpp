/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxHashMapTesterLimP.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/HashBucketLimP.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLimP<>
#define LIBCXX_TEST_BUCKET_NAME "limp"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

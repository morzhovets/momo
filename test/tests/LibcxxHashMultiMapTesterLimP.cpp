/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxHashMultiMapTesterLimP.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MULTI_MAP

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/HashBucketLimP.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLimP<2, momo::MemPoolParams<2>, false>
#define LIBCXX_TEST_BUCKET_NAME "limp"

#include "LibcxxHashMultiMapTester.h"

#endif // TEST_LIBCXX_HASH_MULTI_MAP

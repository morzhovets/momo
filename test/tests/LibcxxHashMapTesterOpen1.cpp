/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxHashMapTesterOpen1.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/HashBucketOpen1.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketOpen1<>
#define LIBCXX_TEST_BUCKET_NAME "open1"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

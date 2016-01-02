/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxHashMapTesterOneI1.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Utility.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketOneI1
#define LIBCXX_TEST_BUCKET_NAME "onei1"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

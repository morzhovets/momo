/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxHashMapTesterOpenN.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/HashBucketOpenN.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketOpenN<>
#define LIBCXX_TEST_BUCKET_NAME "openn"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

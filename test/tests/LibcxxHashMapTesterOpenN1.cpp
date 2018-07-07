/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxHashMapTesterOpenN1.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/HashBucketOpenN1.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketOpenN1<>
#define LIBCXX_TEST_BUCKET_NAME "openn1"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

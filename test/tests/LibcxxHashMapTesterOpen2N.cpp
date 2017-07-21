/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxHashMapTesterOpen2N.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Utility.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketOpen2N<>
#define LIBCXX_TEST_BUCKET_NAME "open2n"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

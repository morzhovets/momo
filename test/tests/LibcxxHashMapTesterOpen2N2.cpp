/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxHashMapTesterOpen2N2.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/HashBucketOpen2N2.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketOpen2N2<>
#define LIBCXX_TEST_BUCKET_NAME "open2n2"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

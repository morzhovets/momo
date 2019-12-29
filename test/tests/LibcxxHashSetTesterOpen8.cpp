/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxHashSetTesterOpen8.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/HashBucketOpen8.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketOpen8
#define LIBCXX_TEST_BUCKET_NAME "open8"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

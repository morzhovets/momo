/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxHashSetTesterOpen1.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET

#undef NDEBUG

#include "../../momo/Utility.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketOpen1<>
#define LIBCXX_TEST_BUCKET_NAME "open1"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

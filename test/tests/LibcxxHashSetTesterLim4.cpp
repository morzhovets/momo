/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxHashSetTesterLim4.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/HashBucketLim4.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLim4<1, 1>
#define LIBCXX_TEST_BUCKET_NAME "lim4"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

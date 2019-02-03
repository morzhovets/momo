/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxHashMapTesterUnlimP.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/HashBucketUnlimP.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketUnlimP<>
#define LIBCXX_TEST_BUCKET_NAME "unlimp"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

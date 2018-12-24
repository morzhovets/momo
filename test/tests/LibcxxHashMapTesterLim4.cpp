/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxHashMapTesterLim4.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/HashBucketLim4.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLim4<>
#define LIBCXX_TEST_BUCKET_NAME "lim4"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxHashMapTesterLim4.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Settings.h"

#undef MOMO_DEFAULT_HASH_BUCKET
#define MOMO_DEFAULT_HASH_BUCKET HashBucketLim4<>

#define LIBCXX_TEST_BUCKET "lim4"

#include "../../momo/details/BucketLim4.h"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

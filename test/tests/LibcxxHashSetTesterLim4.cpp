/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxHashSetTesterLim4.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET

#undef NDEBUG

#include "../../momo/Settings.h"

#undef MOMO_DEFAULT_HASH_BUCKET
#define MOMO_DEFAULT_HASH_BUCKET HashBucketLim4<4, 2>

#define LIBCXX_TEST_BUCKET "lim4"

#include "../../momo/details/BucketLim4.h"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

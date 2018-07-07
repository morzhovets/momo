/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxHashSetTesterOpen2N.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/HashBucketOpen2N.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketOpen2N<1>
#define LIBCXX_TEST_BUCKET_NAME "open2n"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

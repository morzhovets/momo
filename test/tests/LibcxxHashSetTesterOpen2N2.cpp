/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxHashSetTesterOpen2N2.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET

#undef NDEBUG

#include "../../momo/Utility.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketOpen2N2<1, false>
#define LIBCXX_TEST_BUCKET_NAME "open2n2"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxHashSetTesterOneI1.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/HashBucketOneI1.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketOneI1
#define LIBCXX_TEST_BUCKET_NAME "onei1"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxHashMapTesterOneIA.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/HashBucketOneIA.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketOneIA<>
#define LIBCXX_TEST_BUCKET_NAME "oneia"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

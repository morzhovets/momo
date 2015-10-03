/**********************************************************\

  tests/LibcxxHashMapTesterOneI1.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Settings.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketOneI1
#define LIBCXX_TEST_BUCKET_NAME "onei1"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

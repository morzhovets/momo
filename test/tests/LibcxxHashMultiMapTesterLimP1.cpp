/**********************************************************\

  tests/LibcxxHashMultiMapTesterLimP1.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MULTI_MAP

#undef NDEBUG

#include "../../momo/Settings.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLimP1<1, sizeof(void*), 1>
#define LIBCXX_TEST_BUCKET_NAME "limp1"

#include "LibcxxHashMultiMapTester.h"

#endif // TEST_LIBCXX_HASH_MULTI_MAP

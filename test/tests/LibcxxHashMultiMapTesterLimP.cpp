/**********************************************************\

  tests/LibcxxHashMultiMapTesterLimP.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MULTI_MAP

#undef NDEBUG

#include "../../momo/Settings.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLimP<2, 2, false>
#define LIBCXX_TEST_BUCKET_NAME "limp"

#include "LibcxxHashMultiMapTester.h"

#endif // TEST_LIBCXX_HASH_MULTI_MAP

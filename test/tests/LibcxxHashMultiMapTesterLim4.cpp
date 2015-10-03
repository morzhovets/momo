/**********************************************************\

  tests/LibcxxHashMultiMapTesterLim4.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MULTI_MAP

#undef NDEBUG

#include "../../momo/Settings.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLim4<1, 1>
#define LIBCXX_TEST_BUCKET_NAME "lim4"

#include "../../momo/details/BucketLim4.h"

#include "LibcxxHashMultiMapTester.h"

#endif // TEST_LIBCXX_HASH_MULTI_MAP

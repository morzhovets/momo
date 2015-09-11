/**********************************************************\

  tests/LibcxxHashMultiMapTesterLim4.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MULTI_MAP

#undef NDEBUG

#include "../../momo/Settings.h"

#undef MOMO_MAX_ALIGNMENT
#define MOMO_MAX_ALIGNMENT 1

#undef MOMO_DEFAULT_HASH_BUCKET
#define MOMO_DEFAULT_HASH_BUCKET HashBucketLim4<1, 1>

#define LIBCXX_TEST_BUCKET "lim4"

#include "../../momo/HashBuckets/BucketLim4.h"

#include "LibcxxHashMultiMapTester.h"

#endif // TEST_LIBCXX_HASH_MULTI_MAP

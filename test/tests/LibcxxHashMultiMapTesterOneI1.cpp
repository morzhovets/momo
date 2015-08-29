/**********************************************************\

  tests/LibcxxHashMultiMapTesterOneI1.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MULTI_MAP

#undef NDEBUG

#include "../../momo/Settings.h"

#undef MOMO_MAX_ALIGNMENT
#define MOMO_MAX_ALIGNMENT 1

#undef MOMO_DEFAULT_HASH_BUCKET
#define MOMO_DEFAULT_HASH_BUCKET HashBucketOneI1

#define LIBCXX_TEST_BUCKET "onei1"

#include "LibcxxHashMultiMapTester.h"

#endif // TEST_LIBCXX_HASH_MULTI_MAP

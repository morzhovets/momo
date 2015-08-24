/**********************************************************\

  tests/LibcxxHashMultiMapTesterLimP1.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MULTI_MAP

#undef NDEBUG

#include "../../momo/Utility.h"

#undef MOMO_MAX_ALIGNMENT
#define MOMO_MAX_ALIGNMENT (2 * sizeof(void*))

#undef MOMO_DEFAULT_HASH_BUCKET
#define MOMO_DEFAULT_HASH_BUCKET HashBucketLimP1<1, 1, false>

#define LIBCXX_TEST_BUCKET "limp1"

#include "LibcxxHashMultiMapTester.h"

#endif // TEST_LIBCXX_HASH_MULTI_MAP

/**********************************************************\

  tests/LibcxxHashMultiMapTesterFewP.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MULTI_MAP

#undef NDEBUG

#include "../../momo/Utility.h"

#undef MOMO_DEFAULT_HASH_BUCKET
#define MOMO_DEFAULT_HASH_BUCKET HashBucketFewP<1, 0>

#define LIBCXX_TEST_BUCKET "fewp"

#include "../../momo/HashBuckets/BucketFewP.h"

#include "LibcxxHashMultiMapTester.h"

#endif // TEST_LIBCXX_HASH_MULTI_MAP

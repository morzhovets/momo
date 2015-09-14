/**********************************************************\

  tests/LibcxxHashMapTesterFewP.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Settings.h"

#undef MOMO_DEFAULT_HASH_BUCKET
#define MOMO_DEFAULT_HASH_BUCKET HashBucketFewP<1, 32>

#define LIBCXX_TEST_BUCKET "fewp"

#include "../../momo/HashBuckets/BucketFewP.h"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

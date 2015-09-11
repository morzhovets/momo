/**********************************************************\

  tests/LibcxxHashSetTesterLim4.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET

#undef NDEBUG

#include "../../momo/Settings.h"

#undef MOMO_DEFAULT_HASH_BUCKET
#define MOMO_DEFAULT_HASH_BUCKET HashBucketLim4<7, 2>

#define LIBCXX_TEST_BUCKET "lim4"

#include "../../momo/HashBuckets/BucketLim4.h"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

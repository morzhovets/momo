/**********************************************************\

  tests/LibcxxHashMapTesterLimP1.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Utility.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLimP1<>
#define LIBCXX_TEST_BUCKET_NAME "limp1"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

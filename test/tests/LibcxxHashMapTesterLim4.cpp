/**********************************************************\

  tests/LibcxxHashMapTesterLim4.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/BucketLim4.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLim4<>
#define LIBCXX_TEST_BUCKET_NAME "lim4"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

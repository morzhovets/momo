/**********************************************************\

  tests/LibcxxHashSetTesterLim4.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET

#undef NDEBUG

#include "../../momo/Settings.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLim4<4, 2>
#define LIBCXX_TEST_BUCKET_NAME "lim4"

#include "../../momo/details/BucketLim4.h"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

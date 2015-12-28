/**********************************************************\

  tests/LibcxxHashSetTesterLim4.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET

#undef NDEBUG

#include "../../momo/Utility.h"
#include "../../momo/details/BucketLim4.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLim4<1, 1>
#define LIBCXX_TEST_BUCKET_NAME "lim4"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

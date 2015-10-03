/**********************************************************\

  tests/LibcxxHashSetTesterLimP.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET

#undef NDEBUG

#include "../../momo/Utility.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLimP<1, 1>
#define LIBCXX_TEST_BUCKET_NAME "limp"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

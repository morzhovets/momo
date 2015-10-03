/**********************************************************\

  tests/LibcxxHashMultiMapTesterUnlimP.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MULTI_MAP

#undef NDEBUG

#include "../../momo/Utility.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketUnlimP<2, 2>
#define LIBCXX_TEST_BUCKET_NAME "unlimp"

#include "LibcxxHashMultiMapTester.h"

#endif // TEST_LIBCXX_HASH_MULTI_MAP

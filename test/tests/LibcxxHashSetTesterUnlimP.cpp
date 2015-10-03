/**********************************************************\

  tests/LibcxxHashSetTesterUnlimP.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET

#undef NDEBUG

#include "../../momo/Settings.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketUnlimP<1, 1>
#define LIBCXX_TEST_BUCKET_NAME "unlimp"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

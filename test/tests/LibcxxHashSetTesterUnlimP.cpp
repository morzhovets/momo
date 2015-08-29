/**********************************************************\

  tests/LibcxxHashSetTesterUnlimP.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET

#undef NDEBUG

#include "../../momo/Settings.h"

#undef MOMO_MAX_ALIGNMENT
#define MOMO_MAX_ALIGNMENT 1

#undef MOMO_DEFAULT_MEM_MANAGER
#define MOMO_DEFAULT_MEM_MANAGER MemManagerC

#undef MOMO_DEFAULT_HASH_BUCKET
#define MOMO_DEFAULT_HASH_BUCKET HashBucketUnlimP<1, 1>

#define LIBCXX_TEST_BUCKET "unlimp"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

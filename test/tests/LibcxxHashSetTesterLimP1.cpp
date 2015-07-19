/**********************************************************\

  tests/LibcxxHashSetTesterLimP1.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET

#undef NDEBUG

#include "../../momo/Utility.h"

#undef MOMO_DEFAULT_MEM_MANAGER
#define MOMO_DEFAULT_MEM_MANAGER MemManagerC

#undef MOMO_DEFAULT_HASH_BUCKET
#define MOMO_DEFAULT_HASH_BUCKET HashBucketLimP1<2, 0>

#define LIBCXX_TEST_BUCKET "limp1"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

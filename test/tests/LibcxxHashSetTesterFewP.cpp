/**********************************************************\

  tests/LibcxxHashSetTesterFewP.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_SET

#undef NDEBUG

#include "../../momo/Settings.h"

#undef MOMO_MAX_ALIGNMENT
#define MOMO_MAX_ALIGNMENT sizeof(void*)

#undef MOMO_DEFAULT_MEM_MANAGER
#define MOMO_DEFAULT_MEM_MANAGER MemManagerC

#undef MOMO_DEFAULT_HASH_BUCKET
#define MOMO_DEFAULT_HASH_BUCKET HashBucketFewP<1, 1, 1>

#define LIBCXX_TEST_BUCKET "fewp"

#include "../../momo/HashBuckets/BucketFewP.h"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

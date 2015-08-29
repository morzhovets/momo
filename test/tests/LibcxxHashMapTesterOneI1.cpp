/**********************************************************\

  tests/LibcxxHashMapTesterOneI1.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_HASH_MAP

#undef NDEBUG

#include "../../momo/Settings.h"

#undef MOMO_MAX_ALIGNMENT
#define MOMO_MAX_ALIGNMENT sizeof(void*)

#undef MOMO_DEFAULT_MEM_MANAGER
#define MOMO_DEFAULT_MEM_MANAGER MemManagerCpp

#undef MOMO_DEFAULT_HASH_BUCKET
#define MOMO_DEFAULT_HASH_BUCKET HashBucketOneI1

#define LIBCXX_TEST_BUCKET "onei1"

#include "LibcxxHashMapTester.h"

#endif // TEST_LIBCXX_HASH_MAP

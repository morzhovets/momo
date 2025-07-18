/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxHashSetTesterLimP4.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_SET

#include "../../include/momo/details/HashBucketLimP4.h"

#define LIBCXX_TEST_CLASS momo::stdish::unordered_set
#define LIBCXX_TEST_PREFIX_TAIL "limp4"

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxHashSetTesterLimP4_HI.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_SET

#include "../../include/momo/details/HashBucketLimP4.h"

#define LIBCXX_TEST_BUCKET momo::HashBucketLimP4<2>
#define LIBCXX_TEST_PREFIX_TAIL "limp4_hi"
#define LIBCXX_TEST_HINT_ITERATORS

#include "LibcxxHashSetTester.h"

#endif // TEST_LIBCXX_HASH_SET

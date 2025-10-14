/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxHashMultiMapLimP4.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_MULTI_MAP

#include "../../include/momo/details/HashBucketLimP4.h"

#define LIBCXX_TEST_CLASS momo::stdish::unordered_multimap
#define LIBCXX_TEST_PREFIX_TAIL "_limp4"

#include "LibcxxHashMultiMap.h"

#endif // TEST_LIBCXX_HASH_MULTI_MAP

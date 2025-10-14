/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxHashSetOpen8.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_HASH_SET

#include "../../include/momo/details/HashBucketOpen8.h"

#define LIBCXX_TEST_CLASS momo::stdish::unordered_set_open
#define LIBCXX_TEST_PREFIX_TAIL "_open8"

#include "LibcxxHashSet.h"

#endif // TEST_LIBCXX_HASH_SET

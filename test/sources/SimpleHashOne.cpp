/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/SimpleHashOne.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_SIMPLE_HASH
#ifdef TEST_OLD_HASH_BUCKETS

#include "SimpleHash.h"

#include "../../include/momo/details/HashBucketOne.h"

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketOne<>>("momo::HashBucketOne<>");
	SimpleHashTester::TestStrHash<momo::HashBucketOne<sizeof(size_t)>>("momo::HashBucketOne<sizeof(size_t)>");

	SimpleHashTester::TestTemplHashSet<momo::HashBucketOne<1>, 1, 1>("momo::HashBucketOne<1>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOne<4>, 6, 2>("momo::HashBucketOne<4>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOne<2>, 4, 4>("momo::HashBucketOne<2>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOne<sizeof(size_t)>, 8, 8>("momo::HashBucketOne<sizeof(size_t)>");

	return 0;
}();

#endif // TEST_OLD_HASH_BUCKETS
#endif // TEST_SIMPLE_HASH

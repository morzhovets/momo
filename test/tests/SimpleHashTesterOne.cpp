/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/SimpleHashTesterOne.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_SIMPLE_HASH
#ifdef TEST_OLD_HASH_BUCKETS

#include "SimpleHashTester.h"

#include "../../momo/details/HashBucketOne.h"

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketOne<>>("momo::HashBucketOne<>");
	SimpleHashTester::TestStrHash<momo::HashBucketOne<8>>("momo::HashBucketOne<8>");

	SimpleHashTester::TestTemplHashSet<momo::HashBucketOne<1>, 1, 1>("momo::HashBucketOne<1>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOne<4>, 6, 2>("momo::HashBucketOne<4>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOne<2>, 4, 4>("momo::HashBucketOne<2>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOne<8>, 8, 8>("momo::HashBucketOne<8>");

	return 0;
}();

#endif // TEST_OLD_HASH_BUCKETS
#endif // TEST_SIMPLE_HASH

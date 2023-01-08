/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  tests/SimpleHashTesterLimP1.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_SIMPLE_HASH
#ifdef TEST_OLD_HASH_BUCKETS

#include "SimpleHashTester.h"

#include "../../momo/details/HashBucketLimP1.h"

#define BUCKET(maxCount, blockCount) \
	momo::HashBucketLimP1<maxCount, momo::MemPoolParams<blockCount, (blockCount % 2) * 16>>

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketLimP1<>>("momo::HashBucketLimP1<>");
	SimpleHashTester::TestStrHash<momo::HashBucketLimP1<1>>("momo::HashBucketLimP1<1>");

	SimpleHashTester::TestTemplHashSet<BUCKET( 1, 16),  1, 1>("momo::HashBucketLimP1< 1, 16>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 1, 99),  2, 1>("momo::HashBucketLimP1< 1, 99>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 2, 32), 16, 8>("momo::HashBucketLimP1< 2, 32>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 3,  1),  4, 4>("momo::HashBucketLimP1< 3,  1>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 4,  2),  4, 4>("momo::HashBucketLimP1< 4,  2>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 7, 11),  6, 2>("momo::HashBucketLimP1< 7, 11>");
	SimpleHashTester::TestTemplHashSet<BUCKET(15,  1), 11, 1>("momo::HashBucketLimP1<15,  1>");

	return 0;
}();

#endif // TEST_OLD_HASH_BUCKETS
#endif // TEST_SIMPLE_HASH

/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/SimpleHashTesterLimP4.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH

#undef NDEBUG

#include "SimpleHashTester.h"

#include "../../momo/details/HashBucketLimP4.h"

#define BUCKET(maxCount, blockCount) \
	momo::HashBucketLimP4<maxCount, momo::MemPoolParams<blockCount, (blockCount % 2) * 16>>

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketLimP4<>>("momo::HashBucketLimP4<>");

	SimpleHashTester::TestTemplHashSet<BUCKET(1, 16),  1, 1>("momo::HashBucketLimP4<1, 16>");
	SimpleHashTester::TestTemplHashSet<BUCKET(1, 99),  2, 1>("momo::HashBucketLimP4<1, 99>");
	SimpleHashTester::TestTemplHashSet<BUCKET(2, 32), 16, 8>("momo::HashBucketLimP4<2, 32>");
	SimpleHashTester::TestTemplHashSet<BUCKET(3,  1),  4, 4>("momo::HashBucketLimP4<3,  1>");
	SimpleHashTester::TestTemplHashSet<BUCKET(3,  2),  4, 4>("momo::HashBucketLimP4<3,  2>");
	SimpleHashTester::TestTemplHashSet<BUCKET(4, 11),  6, 2>("momo::HashBucketLimP4<4, 11>");
	SimpleHashTester::TestTemplHashSet<BUCKET(4,  1), 11, 1>("momo::HashBucketLimP4<4,  1>");

	return 0;
}();

#endif // TEST_SIMPLE_HASH

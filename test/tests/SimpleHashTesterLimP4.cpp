/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/SimpleHashTesterLimP4.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH

#undef NDEBUG

#include "SimpleHashTester.h"

#define BUCKET(maxCount, blockCount, useHashCodePartGetter) \
	momo::HashBucketLimP4<maxCount, momo::MemPoolParams<blockCount, (blockCount % 2) * 16>, useHashCodePartGetter>

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketLimP4<4, momo::MemPoolParams<>, false>>("momo::HashBucketLimP4<..., false>");
	SimpleHashTester::TestStrHash<momo::HashBucketLimP4<4, momo::MemPoolParams<>,  true>>("momo::HashBucketLimP4<...,  true>");

	SimpleHashTester::TestTemplHashSet<BUCKET(1, 16, false),  1, 1>("momo::HashBucketLimP4<1, 16, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET(1, 99, false),  2, 1>("momo::HashBucketLimP4<1, 99, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET(2, 32, false), 16, 8>("momo::HashBucketLimP4<2, 32, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET(3,  1, false),  4, 4>("momo::HashBucketLimP4<3,  1, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET(3,  2, false),  4, 4>("momo::HashBucketLimP4<3,  2, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET(4, 11, false),  6, 2>("momo::HashBucketLimP4<4, 11, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET(4,  1, false), 11, 1>("momo::HashBucketLimP4<4,  1, false>");

	SimpleHashTester::TestTemplHashSet<BUCKET(1, 16, true),  1, 1>("momo::HashBucketLimP4<1, 16, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET(1, 99, true),  2, 1>("momo::HashBucketLimP4<1, 99, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET(2, 32, true), 16, 8>("momo::HashBucketLimP4<2, 32, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET(3,  1, true),  4, 4>("momo::HashBucketLimP4<3,  1, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET(3,  2, true),  4, 4>("momo::HashBucketLimP4<3,  2, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET(4, 11, true),  6, 2>("momo::HashBucketLimP4<4, 11, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET(4,  1, true), 11, 1>("momo::HashBucketLimP4<4,  1, true>");

	return 0;
}();

#endif // TEST_SIMPLE_HASH

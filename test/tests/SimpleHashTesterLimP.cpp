/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/SimpleHashTesterLimP.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH

#undef NDEBUG

#include "SimpleHashTester.h"

#define BUCKET(maxCount, blockCount, usePtrState) \
	momo::HashBucketLimP<maxCount, momo::MemPoolParams<blockCount, (blockCount % 2) * 16>, usePtrState>

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketLimP<sizeof(void*), momo::MemPoolParams<>, false>>("momo::HashBucketLimP<..., false>");
	SimpleHashTester::TestStrHash<momo::HashBucketLimP<sizeof(void*), momo::MemPoolParams<>,  true>>("momo::HashBucketLimP<...,  true>");

	SimpleHashTester::TestTemplHashSet<BUCKET( 1, 16, false), 11, 1>("momo::HashBucketLimP< 1, 16, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 1, 64, false),  1, 1>("momo::HashBucketLimP< 1, 64, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 2, 99, false),  3, 1>("momo::HashBucketLimP< 2, 99, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 2, 88, false),  2, 2>("momo::HashBucketLimP< 2, 88, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 2,  1, false),  8, 4>("momo::HashBucketLimP< 2,  1, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 3,  2, false),  6, 1>("momo::HashBucketLimP< 3,  2, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 3,  3, false),  8, 2>("momo::HashBucketLimP< 3,  3, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 4,  4, false),  4, 4>("momo::HashBucketLimP< 4,  4, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 5,  5, false), 10, 2>("momo::HashBucketLimP< 5,  5, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 6,  8, false),  4, 4>("momo::HashBucketLimP< 6,  8, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 7, 11, false), 16, 4>("momo::HashBucketLimP< 7, 11, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 8, 33, false),  8, 8>("momo::HashBucketLimP< 8, 33, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET(10, 32, false), 32, 8>("momo::HashBucketLimP<10, 32, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET(12, 55, false), 11, 1>("momo::HashBucketLimP<12, 55, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET(14,  1, false), 16, 8>("momo::HashBucketLimP<14,  1, false>");
	SimpleHashTester::TestTemplHashSet<BUCKET(15, 32, false), 16, 8>("momo::HashBucketLimP<15, 32, false>");

	SimpleHashTester::TestTemplHashSet<BUCKET( 1, 16, true), 11, 1>("momo::HashBucketLimP< 1, 16, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 1, 64, true),  1, 1>("momo::HashBucketLimP< 1, 64, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 2, 99, true),  3, 1>("momo::HashBucketLimP< 2, 99, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 2, 88, true),  2, 2>("momo::HashBucketLimP< 2, 88, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 2,  1, true),  8, 4>("momo::HashBucketLimP< 2,  1, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 3,  2, true),  6, 1>("momo::HashBucketLimP< 3,  2, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 3,  3, true),  8, 2>("momo::HashBucketLimP< 3,  3, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 4,  4, true),  4, 4>("momo::HashBucketLimP< 4,  4, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 5,  5, true), 10, 2>("momo::HashBucketLimP< 5,  5, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 6,  8, true),  4, 4>("momo::HashBucketLimP< 6,  8, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 7, 11, true), 16, 4>("momo::HashBucketLimP< 7, 11, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 8, 33, true),  8, 8>("momo::HashBucketLimP< 8, 33, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET(10, 32, true), 32, 8>("momo::HashBucketLimP<10, 32, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET(12, 55, true), 11, 1>("momo::HashBucketLimP<12, 55, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET(14,  1, true), 16, 8>("momo::HashBucketLimP<14,  1, true>");
	SimpleHashTester::TestTemplHashSet<BUCKET(15, 32, true), 16, 8>("momo::HashBucketLimP<15, 32, true>");

	return 0;
}();

#endif // TEST_SIMPLE_HASH

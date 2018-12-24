/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/SimpleHashTesterLimP1.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH

#undef NDEBUG

#include "SimpleHashTester.h"

#include "../../momo/details/HashBucketLimP1.h"

#define BUCKET(maxCount, blockCount, alignment) \
	momo::HashBucketLimP1<maxCount, momo::MemPoolParams<blockCount, (blockCount % 2) * 16>, alignment>

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketLimP1<>>("momo::HashBucketLimP1<>");

	SimpleHashTester::TestTemplHashSet<BUCKET( 1, 16, 1),  1, 1>("momo::HashBucketLimP1< 1, 16, 1>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 1, 99, 2),  2, 1>("momo::HashBucketLimP1< 1, 99, 2>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 2, 32, 4), 16, 8>("momo::HashBucketLimP1< 2, 32, 4>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 3,  1, 4),  4, 4>("momo::HashBucketLimP1< 3,  1, 4>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 4,  2, 1),  4, 4>("momo::HashBucketLimP1< 4,  2, 1>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 7, 11, 2),  6, 2>("momo::HashBucketLimP1< 7, 11, 2>");
	SimpleHashTester::TestTemplHashSet<BUCKET(15,  1, 4), 11, 1>("momo::HashBucketLimP1<15,  1, 4>");

	return 0;
}();

#endif // TEST_SIMPLE_HASH

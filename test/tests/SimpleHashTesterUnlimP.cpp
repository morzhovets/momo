/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/SimpleHashTesterUnlimP.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH

#undef NDEBUG

#include "SimpleHashTester.h"

#include "../../momo/details/HashBucketUnlimP.h"

#define BUCKET(maxCount, blockCount) \
	momo::HashBucketUnlimP<maxCount, momo::MemPoolParams<blockCount, (blockCount % 2) * 16>>

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketUnlimP<>>("momo::HashBucketUnlimP<>");

	SimpleHashTester::TestTemplHashSet<BUCKET( 1, 32),  1, 1>("momo::HashBucketUnlimP< 1, 32>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 1,  1),  2, 1>("momo::HashBucketUnlimP< 1,  1>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 2, 99), 16, 8>("momo::HashBucketUnlimP< 2, 99>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 3, 16),  4, 4>("momo::HashBucketUnlimP< 3, 16>");
	SimpleHashTester::TestTemplHashSet<BUCKET( 7, 11),  6, 2>("momo::HashBucketUnlimP< 7, 11>");
	SimpleHashTester::TestTemplHashSet<BUCKET(15,  1), 11, 1>("momo::HashBucketUnlimP<15,  1>");

	return 0;
}();

#endif // TEST_SIMPLE_HASH

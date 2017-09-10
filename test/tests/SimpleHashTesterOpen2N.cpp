/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/SimpleHashTesterOpen2N.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH

#undef NDEBUG

#include "SimpleHashTester.h"

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketOpen2N<4, false>>("momo::HashBucketOpen2N<4, false>");
	SimpleHashTester::TestStrHash<momo::HashBucketOpen2N<4,  true>>("momo::HashBucketOpen2N<4,  true>");

	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N<1, false>, 4, 2>("momo::HashBucketOpen2N<1, false>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N<4, false>, 1, 1>("momo::HashBucketOpen2N<4, false>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N<5, false>, 8, 4>("momo::HashBucketOpen2N<5, false>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N<8, false>, 4, 4>("momo::HashBucketOpen2N<8, false>");

	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N<1, true>, 4, 2>("momo::HashBucketOpen2N<1, true>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N<4, true>, 1, 1>("momo::HashBucketOpen2N<4, true>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N<5, true>, 8, 4>("momo::HashBucketOpen2N<5, true>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N<8, true>, 4, 4>("momo::HashBucketOpen2N<8, true>");

	return 0;
}();

#endif // TEST_SIMPLE_HASH

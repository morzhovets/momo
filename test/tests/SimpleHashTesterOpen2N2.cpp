/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/SimpleHashTesterOpen2N2.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH

#undef NDEBUG

#include "SimpleHashTester.h"

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketOpen2N2<3, false>>("momo::HashBucketOpen2N2<3, false>");
	SimpleHashTester::TestStrHash<momo::HashBucketOpen2N2<3,  true>>("momo::HashBucketOpen2N2<3,  true>");

	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N2<1, false>, 4, 2>("momo::HashBucketOpen2N2<1, false>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N2<3, false>, 1, 1>("momo::HashBucketOpen2N2<3, false>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N2<3, false>, 8, 4>("momo::HashBucketOpen2N2<3, false>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N2<2, false>, 4, 4>("momo::HashBucketOpen2N2<2, false>");

	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N2<1, true>, 4, 2>("momo::HashBucketOpen2N2<1, true>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N2<3, true>, 1, 1>("momo::HashBucketOpen2N2<3, true>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N2<3, true>, 8, 4>("momo::HashBucketOpen2N2<3, true>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N2<2, true>, 4, 4>("momo::HashBucketOpen2N2<2, true>");

	return 0;
}();

#endif // TEST_SIMPLE_HASH

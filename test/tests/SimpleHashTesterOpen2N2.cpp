/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/SimpleHashTesterOpen2N2.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH

#undef NDEBUG

#include "SimpleHashTester.h"

#include "../../momo/details/HashBucketOpen2N2.h"

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketOpen2N2<>>("momo::HashBucketOpen2N2<>");

	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N2<1>, 4, 2>("momo::HashBucketOpen2N2<1>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N2<3>, 1, 1>("momo::HashBucketOpen2N2<3>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N2<3>, 8, 4>("momo::HashBucketOpen2N2<3>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N2<2>, 4, 4>("momo::HashBucketOpen2N2<2>");

	return 0;
}();

#endif // TEST_SIMPLE_HASH

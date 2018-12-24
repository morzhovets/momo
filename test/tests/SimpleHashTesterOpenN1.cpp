/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/SimpleHashTesterOpenN1.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH

#undef NDEBUG

#include "SimpleHashTester.h"

#include "../../momo/details/HashBucketOpenN1.h"

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketOpenN1<>>("momo::HashBucketOpenN1<>");

#ifdef MOMO_USE_SSE2
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<true, 1>, 4, 2>("momo::HashBucketOpenN1<true, 1>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<true, 3>, 1, 1>("momo::HashBucketOpenN1<true, 3>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<true, 3>, 8, 4>("momo::HashBucketOpenN1<true, 3>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<true, 7>, 4, 4>("momo::HashBucketOpenN1<true, 7>");
#endif

	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<false, 1, true>, 4, 2>("momo::HashBucketOpenN1<false, 1, true>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<false, 3, true>, 1, 1>("momo::HashBucketOpenN1<false, 3, true>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<false, 3, true>, 8, 4>("momo::HashBucketOpenN1<false, 3, true>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<false, 7, true>, 4, 4>("momo::HashBucketOpenN1<false, 7, true>");

	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<false, 1, false>, 4, 2>("momo::HashBucketOpenN1<false, 1, false>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<false, 3, false>, 1, 1>("momo::HashBucketOpenN1<false, 3, false>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<false, 3, false>, 8, 4>("momo::HashBucketOpenN1<false, 3, false>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<false, 7, false>, 4, 4>("momo::HashBucketOpenN1<false, 7, false>");

	return 0;
}();

#endif // TEST_SIMPLE_HASH

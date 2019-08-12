/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

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
	SimpleHashTester::TestStrHash<momo::HashBucketOpenN1<1>>("momo::HashBucketOpenN1<1>");

	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<1, true>, 4, 2>("momo::HashBucketOpenN1<1, true>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<3, true>, 1, 1>("momo::HashBucketOpenN1<3, true>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<3, true>, 8, 4>("momo::HashBucketOpenN1<3, true>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<7, true>, 4, 4>("momo::HashBucketOpenN1<7, true>");

	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<1, false>, 4, 2>("momo::HashBucketOpenN1<1, false>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<3, false>, 1, 1>("momo::HashBucketOpenN1<3, false>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<3, false>, 8, 4>("momo::HashBucketOpenN1<3, false>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<7, false>, 4, 4>("momo::HashBucketOpenN1<7, false>");

	return 0;
}();

#endif // TEST_SIMPLE_HASH

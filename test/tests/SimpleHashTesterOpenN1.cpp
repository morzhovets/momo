/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/SimpleHashTesterOpenN1.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH

#undef NDEBUG

#include "SimpleHashTester.h"

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketOpenN1<>>("momo::HashBucketOpenN1<>");

	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<1>, 4, 2>("momo::HashBucketOpenN1<1>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<3>, 1, 1>("momo::HashBucketOpenN1<3>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<3>, 8, 4>("momo::HashBucketOpenN1<3>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN1<7>, 4, 4>("momo::HashBucketOpenN1<7>");

	return 0;
}();

#endif // TEST_SIMPLE_HASH

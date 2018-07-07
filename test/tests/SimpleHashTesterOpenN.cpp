/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/SimpleHashTesterOpenN.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH

#undef NDEBUG

#include "SimpleHashTester.h"

#include "../../momo/details/HashBucketOpenN.h"

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketOpenN<>>("momo::HashBucketOpenN<>");

	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN<1>, 4, 2>("momo::HashBucketOpenN<1>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN<4>, 1, 1>("momo::HashBucketOpenN<4>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN<5>, 8, 4>("momo::HashBucketOpenN<5>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpenN<8>, 4, 4>("momo::HashBucketOpenN<8>");

	return 0;
}();

#endif // TEST_SIMPLE_HASH

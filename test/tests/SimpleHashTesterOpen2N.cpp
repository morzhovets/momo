/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/SimpleHashTesterOpen2N.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH

#undef NDEBUG

#include "SimpleHashTester.h"

#include "../../momo/details/HashBucketOpen2N.h"

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketOpen2N<>>("momo::HashBucketOpen2N<>");

	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N<1>, 4, 2>("momo::HashBucketOpen2N<1>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N<4>, 1, 1>("momo::HashBucketOpen2N<4>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N<5>, 8, 4>("momo::HashBucketOpen2N<5>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N<8>, 4, 4>("momo::HashBucketOpen2N<8>");

	return 0;
}();

#endif // TEST_SIMPLE_HASH

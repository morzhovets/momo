/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/SimpleHashTesterOpen2N2.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_SIMPLE_HASH

#include "SimpleHashTester.h"

#include "../../include/momo/details/HashBucketOpen2N2.h"

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketOpen2N2<>>("momo::HashBucketOpen2N2<>");
	SimpleHashTester::TestStrHash<momo::HashBucketOpen2N2<1>>("momo::HashBucketOpen2N2<1>");

	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N2<1>, 4, 2>("momo::HashBucketOpen2N2<1>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N2<3>, 1, 1>("momo::HashBucketOpen2N2<3>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N2<3>, 8, 4>("momo::HashBucketOpen2N2<3>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen2N2<2>, 4, 4>("momo::HashBucketOpen2N2<2>");

	return 0;
}();

#endif // TEST_SIMPLE_HASH

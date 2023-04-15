/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/SimpleHashTesterLim4.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_SIMPLE_HASH
#ifdef TEST_OLD_HASH_BUCKETS

#include "SimpleHashTester.h"

#include "../../include/momo/details/HashBucketLim4.h"

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketLim4<>>("momo::HashBucketLim4<>");
	SimpleHashTester::TestStrHash<momo::HashBucketLim4<1>>("momo::HashBucketLim4<1>");

	SimpleHashTester::TestTemplHashSet<momo::HashBucketLim4<1, 32>,  1, 1>("momo::HashBucketLim4<1, 32>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketLim4<2, 99>, 16, 8>("momo::HashBucketLim4<2, 99>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketLim4<4,  1>, 12, 4>("momo::HashBucketLim4<4,  1>");

	return 0;
}();

#endif // TEST_OLD_HASH_BUCKETS
#endif // TEST_SIMPLE_HASH

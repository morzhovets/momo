/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  tests/SimpleHashTesterOpen8.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_SIMPLE_HASH

#include "SimpleHashTester.h"

#include "../../momo/details/HashBucketOpen8.h"

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketOpen8>("momo::HashBucketOpen8");

	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen8, 4, 2>("momo::HashBucketOpen8");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen8, 1, 1>("momo::HashBucketOpen8");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen8, 8, 4>("momo::HashBucketOpen8");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOpen8, 4, 4>("momo::HashBucketOpen8");

	return 0;
}();

#endif // TEST_SIMPLE_HASH

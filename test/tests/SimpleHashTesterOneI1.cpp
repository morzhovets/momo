/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/SimpleHashTesterOneI1.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH

#undef NDEBUG

#include "SimpleHashTester.h"

#include "../../momo/details/HashBucketOneI1.h"

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketOneI1>("momo::HashBucketOneI1");

	SimpleHashTester::TestTemplHashSet<momo::HashBucketOneI1, 1, 1>("momo::HashBucketOneI1");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOneI1, 4, 2>("momo::HashBucketOneI1");

	return 0;
}();

#endif // TEST_SIMPLE_HASH

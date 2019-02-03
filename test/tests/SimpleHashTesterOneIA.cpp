/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/SimpleHashTesterOneIA.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH

#undef NDEBUG

#include "SimpleHashTester.h"

#include "../../momo/details/HashBucketOneIA.h"

static int testSimpleHash = []
{
	SimpleHashTester::TestStrHash<momo::HashBucketOneIA<>>("momo::HashBucketOneIA<>");

	SimpleHashTester::TestTemplHashSet<momo::HashBucketOneIA<0>, 1, 1>("momo::HashBucketOneIA<0>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOneIA<4>, 6, 2>("momo::HashBucketOneIA<4>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOneIA<2>, 4, 4>("momo::HashBucketOneIA<2>");
	SimpleHashTester::TestTemplHashSet<momo::HashBucketOneIA<8>, 8, 8>("momo::HashBucketOneIA<8>");

	return 0;
}();

#endif // TEST_SIMPLE_HASH

/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/SimpleHashTesterOpenN.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH

#undef NDEBUG

#include "SimpleHashTester.h"

static int testSimpleHash =
	(SimpleHashTester::TestStrHash<momo::HashBucketOpenN<>>("momo::HashBucketOpenN<>"), 0);

#endif // TEST_SIMPLE_HASH

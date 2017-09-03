/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/SimpleHashTesterLim4.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_SIMPLE_HASH

#undef NDEBUG

#include "SimpleHashTester.h"

#include "../../momo/details/HashBucketLim4.h"

static int testSimpleHash =
	(SimpleHashTester::TestStrHash<momo::HashBucketLim4<>>("momo::HashBucketLim4<>"), 0);

#endif // TEST_SIMPLE_HASH

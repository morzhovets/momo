/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/SimpleDataTester.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_SIMPLE_DATA

#undef NDEBUG

#include "../../momo/DataTable.h"

#include <string>
#include <iostream>

class SimpleDataTester
{
public:
	static void TestAll()
	{
	}
};

static int testSimpleData = (SimpleDataTester::TestAll(), 0);

#endif // TEST_SIMPLE_DATA

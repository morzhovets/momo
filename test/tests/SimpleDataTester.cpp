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
private:
	struct Struct
	{
		int intCol;
		double dblCol;
		std::string strCol;
	};

public:
	static void TestAll()
	{
		std::cout << "momo::experimental::DataColumnListStatic: " << std::flush;
		TestData(momo::experimental::DataColumnListStatic<Struct>());
		std::cout << "ok" << std::endl;

		std::cout << "momo::experimental::DataColumnList: " << std::flush;
		typedef momo::experimental::DataColumnList<momo::experimental::DataColumnTraits<Struct>> Columns;
		Columns columns(&Struct::strCol, &Struct::intCol, &Struct::dblCol);
		TestData(columns);
		std::cout << "ok" << std::endl;
	}

	template<typename DataColumnList>
	static void TestData(const DataColumnList& columns)
	{
		typedef momo::experimental::DataTable<DataColumnList> DataTable;
		typedef typename DataTable::Row DataRow;

		DataTable table(columns);

		assert(table.AddUniqueHashIndex(&Struct::intCol, &Struct::strCol));
		assert(table.AddMultiHashIndex(&Struct::intCol));
		assert(table.AddMultiHashIndex(&Struct::strCol));

		for (size_t i = 0; i < 1024; ++i)
		{
			DataRow row = table.NewRow(&Struct::intCol, (int)i / 2);
			row[&Struct::strCol] = (i % 2 == 0) ? "0" : "1";
			row[&Struct::dblCol] = (double)i / 2;
			table.AddRow(std::move(row));
		}
		assert(table.GetCount() == 1024);

		assert(table.HasUniqueHashIndex(&Struct::intCol, &Struct::strCol));
		assert(table.HasMultiHashIndex(&Struct::intCol));
		assert(table.RemoveUniqueHashIndex(&Struct::intCol, &Struct::strCol));
		assert(table.RemoveMultiHashIndex(&Struct::intCol));
		assert(table.AddUniqueHashIndex(&Struct::intCol, &Struct::strCol));
		assert(table.AddMultiHashIndex(&Struct::intCol));
	}
};

static int testSimpleData = (SimpleDataTester::TestAll(), 0);

#endif // TEST_SIMPLE_DATA

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
		TestData(Columns(&Struct::strCol, &Struct::intCol, &Struct::dblCol));
		std::cout << "ok" << std::endl;
	}

	template<typename DataColumnList>
	static void TestData(DataColumnList&& columns)
	{
		typedef momo::experimental::DataTable<DataColumnList> DataTable;
		typedef typename DataTable::Row DataRow;

		static const size_t count = 1024;
		static const size_t count2 = 10;

		columns.SetMutable(&Struct::dblCol);

		DataTable table(std::move(columns));
		const DataTable& ctable = table;

		assert(table.AddUniqueHashIndex(&Struct::intCol, &Struct::strCol));
		assert(table.AddMultiHashIndex(&Struct::intCol));
		assert(table.AddMultiHashIndex(&Struct::strCol));

		for (size_t i = 0; i < count; ++i)
		{
			DataRow row = table.NewRow(&Struct::intCol, (int)i);
			table.AddRow(std::move(row));
		}

		for (size_t i = 0; i < count; ++i)
		{
			DataRow row = table.NewRow(&Struct::intCol, (int)i / 2);
			row[&Struct::strCol] = (i % 2 == 0) ? "0" : "1";
			table.UpdateRow(table[i], std::move(row));
		}

		for (size_t i = 0; i < count; ++i)
			table[i][&Struct::dblCol] = (double)i / 2;

		assert(table.HasUniqueHashIndex(&Struct::intCol, &Struct::strCol));
		assert(table.HasMultiHashIndex(&Struct::intCol));
		assert(table.RemoveUniqueHashIndex(&Struct::intCol, &Struct::strCol));
		assert(table.RemoveMultiHashIndex(&Struct::intCol));
		assert(table.AddUniqueHashIndex(&Struct::intCol, &Struct::strCol));
		assert(table.AddMultiHashIndex(&Struct::intCol));

		for (size_t i = 0; i < count2; ++i)
			table.AddRow(&Struct::intCol, (int)(count + i));
		assert(table.GetCount() == count + count2);

		for (size_t i = 0; i < count2; ++i)
			table.ExtractRow(table[count]);
		assert(table.GetCount() == count);

		assert(table.SelectCount() == count);
		assert(table.Select().GetCount() == count);
		assert(ctable.Select().GetCount() == count);

		assert(table.SelectCount(&Struct::strCol, std::string("0"), &Struct::intCol, 1) == 1);
		assert(table.Select(&Struct::strCol, std::string("1"), &Struct::intCol, 0).GetCount() == 1);
		assert(ctable.Select(&Struct::strCol, std::string("1"), &Struct::intCol, 0).GetCount() == 1);

		assert(table.SelectCount(&Struct::strCol, std::string("0")) == count / 2);
		assert(table.Select(&Struct::strCol, std::string("1")).GetCount() == count / 2);
		assert(ctable.Select(&Struct::strCol, std::string("1")).GetCount() == count / 2);

		assert(table.SelectCount(&Struct::dblCol, 0.0) == 1);
		assert(table.Select(&Struct::dblCol, 1.0).GetCount() == 1);
		assert(ctable.Select(&Struct::dblCol, 1.0).GetCount() == 1);

		DataTable tableCopy(table);
		assert(tableCopy.GetCount() == count);

		table.Clear();
		assert(table.IsEmpty());
	}
};

static int testSimpleData = (SimpleDataTester::TestAll(), 0);

#endif // TEST_SIMPLE_DATA

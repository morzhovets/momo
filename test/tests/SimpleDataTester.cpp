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

namespace
{
	struct Struct
	{
		int intCol;
		double dblCol;
		std::string strCol;
	};

	const auto intCol = &Struct::intCol;
	const auto dblCol = &Struct::dblCol;
	const auto strCol = &Struct::strCol;
}

class SimpleDataTester
{
public:
	static void TestAll()
	{
		std::cout << "momo::experimental::DataColumnListStatic: " << std::flush;
		TestData(momo::experimental::DataColumnListStatic<Struct>());
		std::cout << "ok" << std::endl;

		std::cout << "momo::experimental::DataColumnList: " << std::flush;
		typedef momo::experimental::DataColumnList<momo::experimental::DataColumnTraits<Struct>> Columns;
		TestData(Columns(strCol, intCol, dblCol));
		std::cout << "ok" << std::endl;
	}

	template<typename DataColumnList>
	static void TestData(DataColumnList&& columns)
	{
		typedef momo::experimental::DataTable<DataColumnList> DataTable;
		typedef typename DataTable::Row DataRow;

		static const size_t count = 1024;
		static const size_t count2 = 10;

		columns.SetMutable(dblCol);

		DataTable table(std::move(columns));
		const DataTable& ctable = table;

		assert(table.AddUniqueHashIndex(intCol, strCol));
		assert(table.AddMultiHashIndex(intCol));
		assert(table.AddMultiHashIndex(strCol));

		table.Reserve(count);

		for (size_t i = 0; i < count; ++i)
		{
			DataRow row = table.NewRow(intCol, (int)i);
			assert(table.TryAddRow(std::move(row)).uniqueHashIndex == nullptr);
		}

		for (size_t i = 0; i < count; ++i)
		{
			DataRow row = table.NewRow(intCol, (int)i / 2);
			row[strCol] = (i % 2 == 0) ? "0" : "1";
			assert(table.TryUpdateRow(i, std::move(row)).uniqueHashIndex == nullptr);
		}

		for (size_t i = 0; i < count; ++i)
			table[i][dblCol] = (double)i / 2;

		for (auto row : table) { (void)row; }
		for (auto row : ctable) { (void)row; }
		for (auto row : table.Select()) { (void)row; }
		for (auto row : ctable.Select()) { (void)row; }

		assert(table.GetUniqueHashIndex(intCol, strCol));
		assert(table.GetMultiHashIndex(intCol));
		assert(table.RemoveUniqueHashIndex(intCol, strCol));
		assert(table.RemoveMultiHashIndex(intCol));
		assert(table.AddUniqueHashIndex(intCol, strCol));
		assert(table.AddMultiHashIndex(intCol));

		for (size_t i = 0; i < count2; ++i)
			assert(table.TryInsertRow(count, table.NewRow(intCol, (int)(count + i))).uniqueHashIndex == nullptr);
		assert(table.GetCount() == count + count2);

		for (size_t i = 0; i < count2; ++i)
			table.ExtractRow(table[count]);
		assert(table.GetCount() == count);

		assert(table.SelectCount() == count);
		assert(table.Select().GetCount() == count);
		assert(ctable.Select().GetCount() == count);

		assert(table.SelectCount(strCol, std::string("0"), intCol, 1) == 1);
		assert(table.Select(strCol, std::string("1"), intCol, 0).GetCount() == 1);
		assert(ctable.Select(strCol, std::string("1"), intCol, 0).GetCount() == 1);

		assert(table.SelectCount(strCol, std::string("0")) == count / 2);
		assert(table.Select(strCol, std::string("1")).GetCount() == count / 2);
		assert(ctable.Select(strCol, std::string("1")).GetCount() == count / 2);

		assert(table.SelectCount(dblCol, 0.0) == 1);
		assert(table.Select(dblCol, 1.0).GetCount() == 1);
		assert(ctable.Select(dblCol, 1.0).GetCount() == 1);

		assert(table.FindByUniqueHash(table.GetUniqueHashIndex(intCol, strCol),
			table.NewRow(strCol, std::string("1"), intCol, 0)).GetCount() == 1);
		assert(ctable.FindByUniqueHash(ctable.GetUniqueHashIndex(intCol, strCol),
			table.NewRow(strCol, std::string("1"), intCol, 0)).GetCount() == 1);

		assert(table.FindByUniqueHash(nullptr, strCol, std::string("1"), intCol, 0).GetCount() == 1);
		assert(ctable.FindByUniqueHash(nullptr, strCol, std::string("1"), intCol, 0).GetCount() == 1);

		assert(table.FindByMultiHash(nullptr, strCol, std::string("1")).GetCount() == count / 2);
		assert(ctable.FindByMultiHash(nullptr, strCol, std::string("1")).GetCount() == count / 2);

		DataTable tableCopy(table);
		assert(tableCopy.GetCount() == count);

		table.Clear();
		assert(table.IsEmpty());
	}
};

static int testSimpleData = (SimpleDataTester::TestAll(), 0);

#endif // TEST_SIMPLE_DATA

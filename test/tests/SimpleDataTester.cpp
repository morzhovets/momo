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
		int intStruct;
		double dblStruct;
		std::string strStruct;
	};

	MOMO_DATA_COLUMN_STRUCT(Struct, intStruct);
	MOMO_DATA_COLUMN_STRUCT(Struct, dblStruct);
	MOMO_DATA_COLUMN_STRUCT(Struct, strStruct);

#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning (push)
#pragma warning (disable: 4307)	// integral constant overflow
#endif

	MOMO_DATA_COLUMN_STRING(int, intString);
	MOMO_DATA_COLUMN_STRING(double, dblString);
	MOMO_DATA_COLUMN_STRING(std::string, strString);

#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning (pop)
#endif
}

class SimpleDataTester
{
public:
	static void TestAll()
	{
		std::cout << "momo::experimental::DataColumnListStatic: " << std::flush;
		TestData(momo::experimental::DataColumnListStatic<Struct>(),
			intStruct, dblStruct, strStruct);
		std::cout << "ok" << std::endl;

		std::cout << "momo::experimental::DataColumnList (struct): " << std::flush;
		TestData(momo::experimental::DataColumnList<momo::experimental::DataColumnTraits<Struct>>(
			intStruct, dblStruct, strStruct), intStruct, dblStruct, strStruct);
		std::cout << "ok" << std::endl;

		std::cout << "momo::experimental::DataColumnList (string): " << std::flush;
		TestData(momo::experimental::DataColumnList<>(intString, dblString, strString),
			intString, dblString, strString);
		std::cout << "ok" << std::endl;
	}

	template<typename DataColumnList, typename IntCol, typename DblCol, typename StrCol>
	static void TestData(DataColumnList&& columns,
		const IntCol& intCol, const DblCol& dblCol, const StrCol& strCol)
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
			DataRow row = table.NewRow(intCol = (int)i);
			assert(table.TryAddRow(std::move(row)).uniqueHashIndex == nullptr);
		}

		for (const std::string& s : table.GetColumnItems(strCol))
			assert(s.empty());
		for (const std::string& s : ctable.GetColumnItems(strCol))
			assert(s.empty());

		for (size_t i = 0; i < count; ++i)
		{
			DataRow row = table.NewRow(intCol = (int)i / 2);
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
			assert(table.TryInsertRow(count, table.NewRow(intCol = (int)(count + i))).uniqueHashIndex == nullptr);
		assert(table.GetCount() == count + count2);

		for (size_t i = 0; i < count2; ++i)
			table.ExtractRow(table[count]);
		assert(table.GetCount() == count);

		assert(table.SelectEmpty().IsEmpty());
		assert(ctable.SelectEmpty().IsEmpty());

		auto emptyFilter = [] (typename DataTable::ConstRowReference) { return true; };

		assert(table.SelectCount() == count);
		assert(table.Select().GetCount() == count);
		assert(ctable.Select().GetCount() == count);

		assert(table.SelectCount(strCol == "0", intCol == 1) == 1);
		assert(table.Select(strCol == "1", intCol == 0).GetCount() == 1);
		assert(ctable.Select(strCol == "1", intCol == 0).GetCount() == 1);

		assert(table.SelectCount(emptyFilter, strCol == "0") == count / 2);
		assert(table.Select(emptyFilter, strCol == "1").GetCount() == count / 2);
		assert(ctable.Select(emptyFilter, strCol == "1").GetCount() == count / 2);

		auto selection = table.Select(strCol == "0");
		for (const std::string& s : selection.GetColumnItems(strCol))
			assert(s == "0");

		auto cselection = ctable.Select(strCol == "1");
		for (const std::string& s : cselection.GetColumnItems(strCol))
			assert(s == "1");

		assert(table.SelectCount(dblCol == 0.0) == 1);
		assert(table.Select(dblCol == 1.0).GetCount() == 1);
		assert(ctable.Select(dblCol == 1.0).GetCount() == 1);

		assert(table.FindByUniqueHash(table.GetUniqueHashIndex(intCol, strCol),
			table.NewRow(strCol = "1", intCol = 0))->GetByColumn(intCol) == 0);
		assert(ctable.FindByUniqueHash(ctable.GetUniqueHashIndex(intCol, strCol),
			table.NewRow(strCol = "1", intCol = 0)).GetCount() == 1);

		assert((bool)table.FindByUniqueHash(nullptr, strCol == "1", intCol == 0));
		assert((*ctable.FindByUniqueHash(nullptr, strCol == "1", intCol == 0))[strCol] == "1");

		assert(table.FindByMultiHash(nullptr, strCol == "1").GetCount() == count / 2);
		assert(ctable.FindByMultiHash(nullptr, strCol == "1").GetCount() == count / 2);

		DataTable tableCopy(table);
		assert(tableCopy.GetCount() == count);
		tableCopy = DataTable(table.Select());
		assert(tableCopy.GetCount() == count);
		tableCopy = DataTable(ctable.Select());
		assert(tableCopy.GetCount() == count);

		table.AssignRows(table.GetBegin(), table.GetEnd());
		table.RemoveRows(table.GetBegin(), table.GetBegin());
		table.RemoveRows(emptyFilter);

		assert(table.IsEmpty());
		table.Clear();
	}
};

static int testSimpleData = (SimpleDataTester::TestAll(), 0);

#endif // TEST_SIMPLE_DATA

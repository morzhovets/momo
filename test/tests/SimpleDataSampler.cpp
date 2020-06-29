/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/SimpleDataSampler.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_SIMPLE_DATA

#include "../../momo/DataTable.h"

#include <string>
#include <iostream>

#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning (disable: 4307)	// integral constant overflow
#endif

namespace sample1
{
	MOMO_DATA_COLUMN_STRING(int, intCol);
	MOMO_DATA_COLUMN_STRING(double, dblCol);
	MOMO_DATA_COLUMN_STRING(std::string, strCol);

	void Test()
	{
		// construct empty table with 3 columns
		momo::DataTable<> table(intCol, dblCol, strCol);

		table.AddUniqueHashIndex(strCol, intCol);	// unique index (primary key)

		table.AddRow(strCol = "b", intCol = 1, dblCol = 0.5);
		table.AddRow(intCol = 2, dblCol = 0.5);	// strCol = ""

		table.TryAddRow(intCol = 2);	// not added because of unique index

		std::cout << table.GetCount() << std::endl;	// 2

		// table[0][dblCol] = 1.5;
		table.UpdateRow(table[0], dblCol, 1.5);

		// table[1][strCol] = "a";
		table.UpdateRow(table[1], strCol, std::string("a"));

		for (auto row : table)
			std::cout << row[intCol] << "," << row[dblCol] << "," << row[strCol] << std::endl;
		// 1,1.5,b
		// 2,0.5,a

		{
			// select by condition
			auto selection = table.Select(intCol == 1);
			std::cout << selection.GetCount() << std::endl; // 1

			auto row = selection[0];
			for (auto column : row.GetColumnList())
			{
				if (column.GetTypeInfo() == typeid(double))
					std::cout << row.GetByOffset<double>(column.GetOffset()) << std::endl;
			}
			// 1.5
		}

		{
			// select all and sort
			auto selection = table.Select().Sort(strCol);

			for (const std::string& str : selection.GetColumnItems(strCol))
				std::cout << str << std::endl;
			// a
			// b
		}
	}
}

namespace sample2
{
	struct Struct
	{
		int intCol;
		double dblCol;
		std::string strCol;
	};

	MOMO_DATA_COLUMN_STRUCT(Struct, intCol);
	MOMO_DATA_COLUMN_STRUCT(Struct, dblCol);
	MOMO_DATA_COLUMN_STRUCT(Struct, strCol);

	using ColumnList = momo::DataColumnListStatic<Struct>;
	using Table = momo::DataTable<ColumnList>;

	void Test()
	{
		Table table;

		table.AddMultiHashIndex(strCol);

		{
			auto row = table.NewRow();
			row->intCol = 2;
			row->dblCol = 0.5;
			row->strCol = "a";
			table.AddRow(std::move(row));
		}
		{
			auto row = table.NewRow(intCol = 1, strCol = "b");
			row[dblCol] = 1.5;
			table.InsertRow(0, std::move(row));	// at position 0
		}

		for (auto row : table)
			std::cout << row->intCol << "," << row->dblCol << "," << row->strCol << std::endl;
		// 1,1.5,b
		// 2,0.5,a

		{
			auto selection = table.Select(strCol == "a");	// fast select by index
			std::cout << selection.GetCount() << std::endl;	// 1

			table.RemoveRows(selection.GetBegin(), selection.GetEnd());
			std::cout << table.GetCount() << std::endl;	// 1
		}

		{
			auto selection = table.Select(intCol == 2, dblCol == 0.5);	// slow select (takes linear time)
			std::cout << selection.GetCount() << std::endl;	// 0
		}

#if defined(__cpp_generic_lambdas)
		{
			auto selection = table.Select([] (auto row) { return row[dblCol] > 0.0; });	// slow select
			std::cout << selection.GetCount() << std::endl;	// 1
		}

		table.RemoveRows([] (auto row) { return row[dblCol] > 1.0; });
		std::cout << table.GetCount() << std::endl;	// 0
#endif
	}
}

#endif // TEST_SIMPLE_DATA

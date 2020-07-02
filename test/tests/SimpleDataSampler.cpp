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

		// unique index (primary key)
		table.AddUniqueHashIndex(strCol, intCol);

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
	using Struct = momo::DataStructDefault<int, double, std::string>;
	using ColumnList = momo::DataColumnList<momo::DataColumnTraits<Struct>>;
	using Table = momo::DataTable<ColumnList>;

	MOMO_DATA_COLUMN_STRING_TAG(Struct, int, intCol);
	MOMO_DATA_COLUMN_STRING_TAG(Struct, double, dblCol);
	MOMO_DATA_COLUMN_STRING_TAG(Struct, std::string, strCol);

	void Test()
	{
		// construct empty table with 3 columns
		Table table(intCol, dblCol, strCol);

		// for fast select
		table.AddMultiHashIndex(strCol);

		{
			auto row = table.NewRow(intCol = 1, strCol = "b");
			row[dblCol] = 1.5;
			table.AddRow(std::move(row));
		}
		{
			auto row = table.NewRow();
			row[intCol] = 2;
			row[dblCol] = 0.5;
			row[strCol] = "a";
			table.AddRow(std::move(row));
		}

#if defined(__cpp_generic_lambdas)
		for (auto row : table)
		{
			row.VisitReferences([] (auto& item) { std::cout << item << " "; });
			std::cout << std::endl;
		}
		// 1 1.5 b 
		// 2 0.5 a 

		for (auto row : table)
		{
			auto visitor = [] (auto& item, auto column)
				{ std::cout << column.GetName() << "=" << item << " "; };
			row.VisitReferences(visitor);
			std::cout << std::endl;
		}
		// intCol=1 dblCol=1.5 strCol=b 
		// intCol=2 dblCol=0.5 strCol=a 
#endif

		{
			auto selection = table.Select(strCol == "a");	// fast select by index
			std::cout << selection.GetCount() << std::endl;	// 1

			table.RemoveRows(selection.GetBegin(), selection.GetEnd());
			std::cout << table.GetCount() << std::endl;	// 1
		}

		{
			size_t selCount = table.SelectCount(intCol == 2, dblCol == 0.5);	// slow select (takes linear time)
			std::cout << selCount << std::endl;	// 0
		}

#if defined(__cpp_generic_lambdas)
		{
			// C++11: (Table::ConstRowReference row)
			auto selection = table.Select([] (auto row) { return row[dblCol] > 0.0; });	// slow select
			std::cout << selection.GetCount() << std::endl;	// 1
		}

		table.RemoveRows([] (auto row) { return row[dblCol] > 1.0; });
		std::cout << table.GetCount() << std::endl;	// 0
#endif
	}
}

namespace sample3
{
	struct Struct
	{
		int intCol;
		double dblCol;
		std::string strCol;
	};

	using ColumnList = momo::DataColumnListStatic<Struct>;
	using Table = momo::DataTable<ColumnList>;

	MOMO_DATA_COLUMN_STRUCT(Struct, intCol);
	MOMO_DATA_COLUMN_STRUCT(Struct, dblCol);
	MOMO_DATA_COLUMN_STRUCT(Struct, strCol);

	void Test()
	{
		// construct empty table with 3 columns
		Table table;

		{
			auto row = table.NewRow();
			row->intCol = 2;
			row->dblCol = 0.5;
			row->strCol = "a";
			table.AddRow(std::move(row));
		}
		{
			auto row = table.NewRow(intCol = 1, strCol = "a");
			row[dblCol] = 1.5;
			table.InsertRow(0, std::move(row));	// at position 0
		}

		for (auto row : table)
			std::cout << row->intCol << "," << row->dblCol << "," << row->strCol << std::endl;
		// 1,1.5,a
		// 2,0.5,a

		auto uniqueIndex = table.AddUniqueHashIndex(strCol, intCol);
		auto multiIndex = table.AddMultiHashIndex(strCol);

		{
			auto rows = table.FindByMultiHash(multiIndex, strCol == "a");	// fastest search
			std::cout << rows.GetCount() << std::endl;
			// 2
		}

		{
			auto prow = table.FindByUniqueHash(uniqueIndex, intCol == 1, strCol == "a");	// fastest search
			if (prow)
				std::cout << (*prow)[dblCol] << std::endl;
			// 1.5
		}

		table.RemoveRow(table[0]);
		std::cout << table[0][dblCol] << std::endl;
		// 0.5
	}
}

#endif // TEST_SIMPLE_DATA

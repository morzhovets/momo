/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/SimpleDataSampler.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_SIMPLE_DATA

#include "../../momo/DataTable.h"

#include <string>
#include <iostream>

#ifdef TEST_MSVC
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
		momo::DataTable<> table({ intCol, dblCol, strCol });

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
			std::cout << row[intCol] << " " << row[dblCol] << " " << row[strCol] << std::endl;
		// 1 1.5 b
		// 2 0.5 a

		{
			// select by condition
			auto selection = table.Select(intCol == 1);
			std::cout << selection.GetCount() << std::endl; // 1
			std::cout << selection[0][dblCol] << std::endl;	// 1.5
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
	using Table = momo::DataTable<>;
	using ConstRowReference = Table::ConstRowReference;

	template<typename Item>
	using Column = Table::Column<Item>;

	// unique column codes: 0, 1, 2
	// column names are optional
	constexpr Column<int> intCol(uint64_t{0}, "intCol");
	constexpr Column<double> dblCol(uint64_t{1}, "dblCol");
	constexpr Column<std::string> strCol(uint64_t{2}, "strCol");

	void Test()
	{
		// construct empty table with 3 columns
		Table table({ intCol, dblCol, strCol });

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

		for (auto row : table)
		{
			for (auto column : table.GetColumnList())
			{
				const std::type_info& typeInfo = column.GetTypeInfo();
				size_t offset = column.GetOffset();
				if (typeInfo == typeid(int))
					std::cout << row.GetByOffset<int>(offset) << " ";
				else if (typeInfo == typeid(double))
					std::cout << row.GetByOffset<double>(offset) << " ";
				else if (typeInfo == typeid(std::string))
					std::cout << row.GetByOffset<std::string>(offset) << " ";
			}
			std::cout << std::endl;
		}
		// 1 1.5 b 
		// 2 0.5 a 

		{
			auto selection = table.Select(strCol == "a");	// fast select by index
			std::cout << selection.GetCount() << std::endl;	// 1

			table.RemoveRows(selection.GetBegin(), selection.GetEnd());
			std::cout << table.GetCount() << std::endl;	// 1
		}

		{
			size_t count = table.SelectCount(intCol == 2, dblCol == 0.5);	// slow select (takes linear time)
			std::cout << count << std::endl;	// 0
		}

		{
			auto selection = table.Select([] (ConstRowReference row) { return row[dblCol] > 0.0; });	// slow select
			std::cout << selection.GetCount() << std::endl;	// 1
		}

		table.RemoveRows([] (ConstRowReference row) { return row[dblCol] > 1.0; });
		std::cout << table.GetCount() << std::endl;	// 0
	}
}

namespace sample3
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
		Table table({ intCol, dblCol, strCol });

		table.AddRow(intCol = 2, dblCol = 0.5, strCol = "a");
		table.InsertRow(0, intCol = 1, dblCol = 1.5, strCol = "b");	// at position 0

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

		table.RemoveRow(table[0]);
		std::cout << table[0][dblCol] << std::endl;	// 0.5
	}
}

namespace sample4
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
			row->intCol = 1;
			row->dblCol = 1.5;
			row->strCol = "a";
			table.AddRow(std::move(row));
		}
		{
			auto row = table.NewRow(intCol = 2, strCol = "a");
			row[dblCol] = 0.5;
			table.AddRow(std::move(row));
		}

		for (auto row : table)
			std::cout << row->intCol << " " << row->dblCol << " " << row->strCol << std::endl;
		// 1 1.5 a
		// 2 0.5 a

		auto uniqueIndex = table.AddUniqueHashIndex(strCol, intCol);
		auto multiIndex = table.AddMultiHashIndex(strCol);

		{
			auto prow = table.FindByUniqueHash(uniqueIndex, intCol == 1, strCol == "a");	// fastest search
			std::cout << (*prow)[dblCol] << std::endl;	// 1.5
		}

		{
			auto rows = table.FindByMultiHash(multiIndex, strCol == "a");	// fastest search
			std::cout << rows.GetCount() << std::endl;	// 2
		}
	}
}

#endif // TEST_SIMPLE_DATA

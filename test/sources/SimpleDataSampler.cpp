/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/SimpleDataSampler.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_SIMPLE_DATA

#if defined(TEST_MSVC) && _MSC_VER < 1939
#pragma warning (disable: 4307)	// integral constant overflow
#endif

#include "SimpleDataSampler.h"

#include <iostream>
#include <sstream>

namespace sample_data1
{
// Declarations in SimpleDataSampler.h
/*
	inline constexpr momo::DataColumn<int> intCol("intCol");
	inline constexpr momo::DataColumn<double> dblCol("dblCol");
	inline constexpr momo::DataColumn<std::string> strCol("strCol");
*/
	void Sample(std::ostream& output)
	{
		// in this sample the list of columns is specified when constructing the DataTable
		momo::DataTable<> table({ intCol, dblCol, strCol });	// construct empty table with 3 columns

		// unique index (primary key)
		table.AddUniqueHashIndex(strCol, intCol);

		table.AddRow(strCol = "b", intCol = 1, dblCol = 0.5);
		table.AddRow(intCol = 2, dblCol = 2.5);	// strCol = ""

		if (!table.TryAddRow(intCol = 2))
		{
			// not added because of unique index (intCol == 2, strCol == "")
			output << "!" << std::endl;	// !
		}

		output << table.GetCount() << std::endl;	// 2

		// `table[0][dblCol]` returns `const double&`
		output << table[0][dblCol] << std::endl;	// 0.5

		// after adding the row can be modified by function `UpdateRow`
		table.UpdateRow(table[0], dblCol, 1.5);
		table.UpdateRow(table[1], strCol, "a");

		for (auto row : table)
			output << row[intCol] << " " << row[dblCol] << " " << row[strCol] << std::endl;
		// 1 1.5 b
		// 2 2.5 a

		{
			// select by conditions
			auto selection = table.Select(intCol == 1 && dblCol == 1.5);
			output << selection.GetCount() << std::endl;	// 1
			output << selection[0][strCol] << std::endl;	// b
		}

		{
			// select all and sort
			auto selection = table.Select().Sort(strCol);

			for (const std::string& str : selection.GetColumnItems(strCol))
				output << str << std::endl;
			// a
			// b
		}
	}
}

namespace sample_data2
{
// Declarations in SimpleDataSampler.h
/*
	struct Struct
	{
		int intCol;
		double dblCol;
		std::string strCol;
	};
*/
	void Sample(std::ostream& output)
	{
		// in this sample the list of columns is specified in accordance with the `Struct` struct
		momo::DataTableNative<Struct> table;	// construct empty table with 3 columns

		// unique index (primary key)
		table.AddUniqueHashIndex(&Struct::strCol, &Struct::intCol);

#if defined(__cpp_designated_initializers)	// C++20
		table.AddRow(table.NewRow({ .intCol = 1, .dblCol = 0.5, .strCol = "b" }));
#else
		table.AddRow(table.NewRow({ /*.intCol =*/ 1, /*.dblCol =*/ 0.5, /*.strCol =*/ "b" }));
#endif

		{
			auto row = table.NewRow();
			row->intCol = 2;
			row->dblCol = 2.5;
			table.AddRow(std::move(row));	// strCol = ""
		}

		if (!table.TryAddRow(table.NewRow({ /*.intCol =*/ 1, /*.dblCol =*/ 4.5, /*.strCol =*/ "b" })))
		{
			// not added because of unique index (intCol == 1, strCol == "b")
			output << "!" << std::endl;	// !
		}

		output << table.GetCount() << std::endl;	// 2

		// `table[0]->dblCol` has type `const double&`
		output << table[0]->dblCol << std::endl;	// 0.5

		// after adding the row can be modified by function `UpdateRow`
		table.UpdateRow(table[0], &Struct::dblCol, 1.5);
		table.UpdateRow(table[1], &Struct::strCol, "a");

		for (auto row : table)
			output << row->intCol << " " << row->dblCol << " " << row->strCol << std::endl;
		// 1 1.5 b
		// 2 2.5 a

		{
			// select by conditions
#if defined(MOMO_HAS_DEDUCTION_GUIDES)
			auto selection = table.Select(
				momo::DataEquality(&Struct::intCol, 1).And(&Struct::dblCol, 1.5));
#else
			auto selection = table.Select(
				momo::DataEquality<>().And(&Struct::intCol, 1).And(&Struct::dblCol, 1.5));
#endif
			output << selection.GetCount() << std::endl;	// 1
			output << selection[0]->strCol << std::endl;	// b
		}

		{
			// select all and sort
			auto selection = table.Select().Sort(&Struct::strCol);

			for (const std::string& str : selection.GetColumnItems(&Struct::strCol))
				output << str << std::endl;
			// a
			// b
		}
	}
}

namespace sample_data3
{
// Declarations in SimpleDataSampler.h
/*
	using Table = momo::DataTable<>;
	using ConstRowReference = Table::ConstRowReference;

	template<typename Item>
	using Column = Table::Column<Item>;

	inline constexpr Column<int> intCol("intCol");
	inline constexpr Column<double> dblCol("dblCol");
	inline constexpr Column<std::string> strCol("strCol");
*/
	void Sample(std::ostream& output)
	{
		// construct empty table with 3 columns
		Table table({ intCol, dblCol, strCol });

		// non-unique index for fast select
		table.AddMultiHashIndex(strCol);

		{
			auto row = table.NewRow(intCol = 1, strCol = "a");
			row[dblCol] = 1.5;
			table.AddRow(std::move(row));
		}
		{
			auto row = table.NewRow(table[0]);	// copy previous row
			row[intCol] = 2;
			row[dblCol] = 2.5;
			table.AddRow(std::move(row));	// strCol = "a"
		}

#if !defined(MOMO_DISABLE_TYPE_INFO)
		for (auto row : table)
		{
			for (auto column : table.GetColumnList())
			{
				const std::type_info& typeInfo = column.GetTypeInfo();
				size_t offset = column.GetOffset();
				if (typeInfo == typeid(int))
					output << row.GetByOffset<int>(offset) << " ";
				else if (typeInfo == typeid(double))
					output << row.GetByOffset<double>(offset) << " ";
				else if (typeInfo == typeid(std::string))
					output << row.GetByOffset<std::string>(offset) << " ";
			}
			output << std::endl;
		}
		// 1 1.5 a 
		// 2 2.5 a 
#endif

		{
			size_t count = table.SelectCount(strCol == "a");	// fast select by index
			output << count << std::endl;	// 2
		}

		{
			auto selection = table.Select(intCol == 1 && dblCol == 1.5);	// slow select (takes linear time)
			output << selection.GetCount() << std::endl;	// 1
		}

		{
			auto selection = table.Select(
				[] (ConstRowReference row) { return row[dblCol] > 2.0; });	// slow select
			output << selection.GetCount() << std::endl;	// 1

			table.RemoveRows(selection.GetBegin(), selection.GetEnd());
			output << table.GetCount() << std::endl;	// 1
		}

		table.RemoveRows([] (ConstRowReference row) { return row[dblCol] > 1.0; });
		output << table.GetCount() << std::endl;	// 0
	}
}

namespace sample_data4
{
// Declarations in SimpleDataSampler.h
/*
	using Struct = momo::DataStructDefault<int, double, std::string>;
	using ColumnList = momo::DataColumnList<momo::DataColumnTraits<Struct>>;
	using Table = momo::DataTable<ColumnList>;

	template<typename Item>
	using Column = Table::Column<Item>;

	inline constexpr Column<int> intCol("intCol");
	inline constexpr Column<double> dblCol("dblCol");
	inline constexpr Column<std::string> strCol("strCol");
*/
	void Sample(std::ostream& output)
	{
		// construct empty table with 3 columns
		Table table({ intCol, dblCol, strCol });

		table.AddRow(intCol = 2, dblCol = 0.5, strCol = "a");
		table.InsertRow(0, intCol = 1, dblCol = 1.5, strCol = "b");	// at position 0

#if !defined(MOMO_DISABLE_TYPE_INFO) && defined(__cpp_generic_lambdas)
		for (auto row : table)
		{
			row.VisitReferences([&output] (auto& item) { output << item << " "; });
			output << std::endl;
		}
		// 1 1.5 b 
		// 2 0.5 a 

		for (auto row : table)
		{
			auto visitor = [&output] (auto& item, auto column)
				{ output << column.GetName() << "=" << item << " "; };
			row.VisitReferences(visitor);
			output << std::endl;
		}
		// intCol=1 dblCol=1.5 strCol=b 
		// intCol=2 dblCol=0.5 strCol=a 
#endif

		table.RemoveRow(table[0]);
		output << table[0][dblCol] << std::endl;	// 0.5
	}
}

namespace sample_data5
{
// Declarations in SimpleDataSampler.h
/*
	struct Struct
	{
		// initialize fields to avoid Wmissing-field-initializers
		std::string strCol{};
		int intCol{};
		double dblCol{};
	};

	using Table = momo::DataTableNative<Struct>;
*/
	void Sample(std::ostream& output)
	{
		// construct empty table with 3 columns
		Table table;

#if defined(__cpp_designated_initializers)	// C++20
		table.AddRow(table.NewRow({ .strCol = "a", .intCol = 1, .dblCol = 1.5 }));
		table.AddRow(table.NewRow({ .strCol = "a", .intCol = 2 }));
#else
		table.AddRow(table.NewRow({ /*.strCol =*/ "a", /*.intCol =*/ 1, /*.dblCol =*/ 1.5 }));
		table.AddRow(table.NewRow({ /*.strCol =*/ "a", /*.intCol =*/ 2 }));
#endif

		for (auto row : table)
			output << row->strCol << " " << row->intCol << " " << row->dblCol << std::endl;
		// a 1 1.5
		// a 2 0

		auto uniqueIndex = table.AddUniqueHashIndex(&Struct::strCol, &Struct::intCol);
		auto multiIndex = table.AddMultiHashIndex(&Struct::strCol);

		{
			auto prow = table.FindByUniqueHash(
				momo::DataEquality<>().And(&Struct::intCol, 1).And(&Struct::strCol, "a"),
				uniqueIndex);	// fastest search
			output << (*prow)->dblCol << std::endl;	// 1.5
		}

		{
			auto rows = table.FindByMultiHash(
				momo::DataEquality<>().And(&Struct::strCol, "a"), multiIndex);	// fastest search
			output << rows.GetCount() << std::endl;	// 2
		}
	}
}

static int sampleData = []
{
	std::cout << "momo::DataTable samples: " << std::flush;

	std::stringstream resStream;
	sample_data1::Sample(resStream);
	sample_data2::Sample(resStream);
	sample_data3::Sample(resStream);
	sample_data4::Sample(resStream);
	sample_data5::Sample(resStream);

	std::string res1 = "!\n2\n0.5\n1 1.5 b\n2 2.5 a\n1\nb\na\nb\n";
	std::string res2 = "!\n2\n0.5\n1 1.5 b\n2 2.5 a\n1\nb\na\nb\n";
	std::string res3 = "2\n1\n1\n1\n0\n";
	std::string res4 = "0.5\n";
	std::string res5 = "a 1 1.5\na 2 0\n1.5\n2\n";
#if !defined(MOMO_DISABLE_TYPE_INFO)
	res3 = "1 1.5 a \n2 2.5 a \n" + res3;
#endif
#if !defined(MOMO_DISABLE_TYPE_INFO) && defined(__cpp_generic_lambdas)
	res4 = "1 1.5 b \n2 0.5 a \nintCol=1 dblCol=1.5 strCol=b \nintCol=2 dblCol=0.5 strCol=a \n" + res4;
#endif

	assert(resStream.str() == res1 + res2 + res3 + res4 + res5);

	std::cout << "ok" << std::endl;
	return 0;
}();

#endif // TEST_SIMPLE_DATA

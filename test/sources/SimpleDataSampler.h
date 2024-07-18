/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/SimpleDataSampler.h

\**********************************************************/

#pragma once

#include "../../include/momo/DataTable.h"

#include <string>

namespace sample_data1
{
#if defined(__cpp_inline_variables)	// C++17
	inline constexpr momo::DataColumn<int> intCol("intCol");
	inline constexpr momo::DataColumn<double> dblCol("dblCol");
	inline constexpr momo::DataColumn<std::string> strCol("strCol");
#else
	constexpr momo::DataColumn<int> intCol("intCol");
	constexpr momo::DataColumn<double> dblCol("dblCol");
	constexpr momo::DataColumn<std::string> strCol("strCol");
#endif
}

namespace sample_data2
{
	struct Struct
	{
		int intCol;
		double dblCol;
		std::string strCol;
	};
}

namespace sample_data3
{
	using Table = momo::DataTable<>;
	using ConstRowReference = Table::ConstRowReference;

	template<typename Item>
	using Column = Table::Column<Item>;

#if defined(__cpp_inline_variables)	// C++17
	inline constexpr Column<int> intCol("intCol");
	inline constexpr Column<double> dblCol("dblCol");
	inline constexpr Column<std::string> strCol("strCol");
#else
	constexpr Column<int> intCol("intCol");
	constexpr Column<double> dblCol("dblCol");
	constexpr Column<std::string> strCol("strCol");
#endif
}

namespace sample_data4
{
	using Struct = momo::DataStructDefault<int, double, std::string>;
	using ColumnList = momo::DataColumnList<momo::DataColumnTraits<Struct>>;
	using Table = momo::DataTable<ColumnList>;

	template<typename Item>
	using Column = Table::Column<Item>;

#if defined(__cpp_inline_variables)	// C++17
	inline constexpr Column<int> intCol("intCol");
	inline constexpr Column<double> dblCol("dblCol");
	inline constexpr Column<std::string> strCol("strCol");
#else
	constexpr Column<int> intCol("intCol");
	constexpr Column<double> dblCol("dblCol");
	constexpr Column<std::string> strCol("strCol");
#endif
}

namespace sample_data5
{
	struct Struct
	{
		int intCol;
		double dblCol;
		std::string strCol;
	};

	using Table = momo::DataTableNative<Struct>;
	using ColumnInfo = Table::ColumnList::ColumnInfo;
}

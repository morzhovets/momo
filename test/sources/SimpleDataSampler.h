/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/SimpleDataSampler.h

\**********************************************************/

#pragma once

#include "../../include/momo/DataTable.h"

#include <string>

namespace sample_data1
{
	using Table = momo::DataTable<>;

	template<typename Item>
	using Column = Table::Column<Item>;

	inline constexpr Column<int> intCol("intCol");
	inline constexpr Column<double> dblCol("dblCol");
	inline constexpr Column<std::string> strCol("strCol");
}

namespace sample_data2
{
	using Table = momo::DataTable<>;

	template<typename Item>
	using Column = Table::Column<Item>;

	inline constexpr Column<int> intCol("intCol");
	inline constexpr Column<double> dblCol("dblCol");
	inline constexpr Column<std::string> strCol("strCol");
}

namespace sample_data3
{
	using Struct = momo::DataStructDefault<int, double, std::string>;
	using ColumnList = momo::DataColumnList<momo::DataColumnTraits<Struct>>;
	using Table = momo::DataTable<ColumnList>;

	template<typename Item>
	using Column = Table::Column<Item>;

	inline constexpr Column<int> intCol("intCol");
	inline constexpr Column<double> dblCol("dblCol");
	inline constexpr Column<std::string> strCol("strCol");
}

namespace sample_data4
{
	struct Struct
	{
		int intCol;
		double dblCol;
		std::string strCol;
	};

	using ColumnList = momo::DataColumnListStatic<Struct>;
	using Table = momo::DataTable<ColumnList>;

	inline MOMO_DATA_COLUMN_STRUCT(Struct, intCol);
	inline MOMO_DATA_COLUMN_STRUCT(Struct, dblCol);
	inline MOMO_DATA_COLUMN_STRUCT(Struct, strCol);
}

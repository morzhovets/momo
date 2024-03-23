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
	using Table = momo::DataTable<>;

	template<typename Item>
	using Column = Table::Column<Item>;

#if defined(__cpp_inline_variables)	// C++17
	inline constexpr Column<int> intCol("intCol");
	inline constexpr Column<double> dblCol("dblCol");
	inline constexpr Column<std::string> strCol("strCol");
#else
	struct Cols	// inlined constants in C++11/14
	{
		static constexpr Column<int> intCol{"intCol"};
		static constexpr Column<double> dblCol{"dblCol"};
		static constexpr Column<std::string> strCol{"strCol"};
	};
#endif
}

namespace sample_data2
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
	struct Cols	// inlined constants in C++11/14
	{
		static constexpr Column<int> intCol{"intCol"};
		static constexpr Column<double> dblCol{"dblCol"};
		static constexpr Column<std::string> strCol{"strCol"};
	};
#endif
}

namespace sample_data3
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
	struct Cols	// inlined constants in C++11/14
	{
		static constexpr Column<int> intCol{"intCol"};
		static constexpr Column<double> dblCol{"dblCol"};
		static constexpr Column<std::string> strCol{"strCol"};
	};
#endif
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

#if defined(__cpp_inline_variables)	// C++17
	inline MOMO_DATA_COLUMN_STRUCT(Struct, intCol);
	inline MOMO_DATA_COLUMN_STRUCT(Struct, dblCol);
	inline MOMO_DATA_COLUMN_STRUCT(Struct, strCol);
#else
	struct Cols	// inlined constants in C++11/14
	{
		static MOMO_DATA_COLUMN_STRUCT(Struct, intCol);
		static MOMO_DATA_COLUMN_STRUCT(Struct, dblCol);
		static MOMO_DATA_COLUMN_STRUCT(Struct, strCol);
	};
#endif
}

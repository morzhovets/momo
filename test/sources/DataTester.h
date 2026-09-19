/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/DataTester.h

\**********************************************************/

#pragma once

#include "TestSettings.h"

#include "../../include/momo/DataTable.h"

#if defined(TEST_GCC) && __GNUC__ < 6
namespace std
{
	template<>
	struct hash<momo::DataColumnCodeOffset>
	{
		size_t operator()(const momo::DataColumnCodeOffset& key) const noexcept
		{
			return std::hash<size_t>()(static_cast<size_t>(key));
		}
	};
}
#endif

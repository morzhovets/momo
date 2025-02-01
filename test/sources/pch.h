/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/pch.h

\**********************************************************/

#pragma once

#include "TestSettings.h"

#ifdef _WIN32
# ifdef TEST_LIBCXX_NEW
#  define NOMINMAX
# endif
# include <Windows.h>
# define MOMO_USE_MEM_MANAGER_WIN
#endif

#include "../../include/momo/Utility.h"

#include <string>
#include <iostream>
#include <random>

#include "LibcxxTester.h"	//?

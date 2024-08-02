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

#ifdef TEST_MSVC
# define _SCL_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
# pragma warning (disable: 4127)	// conditional expression is constant
# pragma warning (disable: 4458)	// declaration of '...' hides class member
# if _MSC_VER == 1900
#  pragma warning (disable: 4503)	// decorated name length exceeded, name was truncated
#  pragma warning (disable: 4702)	// unreachable code
# endif
#endif

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

/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/pch.h

\**********************************************************/

#pragma once

#include "TestSettings.h"

#ifndef TEST_SPEED_MAP
#undef NDEBUG
#endif

#ifdef TEST_MSVC
#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma warning (disable: 4127)	// conditional expression is constant
#pragma warning (disable: 4458)	// declaration of '...' hides class member
#endif

#ifdef _WIN32
#define MOMO_USE_MEM_MANAGER_WIN
#include <Windows.h>
#endif

#include "../../include/momo/Utility.h"

#include <version>
#include <string>
#include <iostream>
#include <random>

#include "LibcxxTester.h"

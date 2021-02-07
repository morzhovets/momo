/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/pch.h

\**********************************************************/

#pragma once

#if defined(_MSC_VER) && !defined(__clang__)
#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#pragma warning (disable: 4127)	// conditional expression is constant
#pragma warning (disable: 4458)	// declaration of '...' hides class member
#endif

#include "TestSettings.h"

#include "../../momo/Utility.h"

#include <string>
#include <iostream>
#include <random>

#include "LibcxxTester.h"

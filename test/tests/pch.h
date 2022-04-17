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
#if _MSC_VER == 1900
#pragma warning (disable: 4503)	// decorated name length exceeded, name was truncated
#pragma warning (disable: 4702)	// unreachable code
#endif
#endif

#include "TestSettings.h"

#include "../../momo/Utility.h"

#include <string>
#include <iostream>
#include <random>

#include "LibcxxTester.h"

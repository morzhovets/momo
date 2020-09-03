/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/pch.h

\**********************************************************/

//#pragma once
#ifndef PCH_INCLUDED
#define PCH_INCLUDED

#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning (disable: 4127)	// conditional expression is constant
#pragma warning (disable: 4458)	// declaration of '...' hides class member
#pragma warning (disable: 4503)	// decorated name length exceeded, name was truncated
#define _SCL_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <cstring>
#include <memory>
#include <exception>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <utility>
#include <array>
#include <initializer_list>

#ifdef _MSC_VER //_WIN32
#include <Windows.h>
#endif

#include <string>
#include <iostream>
#include <random>

#include "LibcxxTester.h"

#endif // PCH_INCLUDED

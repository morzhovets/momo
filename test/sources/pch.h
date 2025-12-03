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

#include "../../include/momo/Utility.h"

#include <version>
#include <atomic>

#ifdef MOMO_USE_MEM_MANAGER_WIN
# include <Windows.h>
#endif

#include <iostream>
#include <sstream>
#include <string>
#include <cerrno>
#include <cmath>
#include <limits>
#include <climits>
#include <cfloat>
#include <random>
#include <optional>
#include <span>

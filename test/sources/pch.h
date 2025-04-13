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

#ifdef MOMO_USE_MEM_MANAGER_WIN
# include <Windows.h>
#endif

#include <version>
#include <string>
#include <iostream>
#include <random>

#include "../../include/momo/Utility.h"

#include "LibcxxTester.h"	//?

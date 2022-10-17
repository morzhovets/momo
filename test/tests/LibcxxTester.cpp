/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxTester.cpp

\**********************************************************/

#include "pch.h"

#include "LibcxxTester.h"

int DefaultOnly::count = 0;

int Counter_base::gConstructed = 0;

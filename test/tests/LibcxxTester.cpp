/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxTester.cpp

\**********************************************************/

#include "../../momo/Utility.h"

#include "LibcxxTester.h"

int DefaultOnly::count = 0;

int Counter_base::gConstructed = 0;

int test_alloc_base::count = 0;
int test_alloc_base::time_to_throw = 0;
int test_alloc_base::alloc_count = 0;
int test_alloc_base::throw_after = INT_MAX;
int test_alloc_base::copied = 0;
int test_alloc_base::moved = 0;
int test_alloc_base::converted = 0;

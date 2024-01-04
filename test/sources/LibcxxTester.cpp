/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxTester.cpp

\**********************************************************/

#include "pch.h"

#include "LibcxxTester.h"

#ifndef TEST_LIBCXX_NEW

int DefaultOnly::count = 0;

int Counter_base::gConstructed = 0;

int test_alloc_base::count = 0;
int test_alloc_base::time_to_throw = 0;
int test_alloc_base::alloc_count = 0;
int test_alloc_base::throw_after = INT_MAX;
int test_alloc_base::copied = 0;
int test_alloc_base::moved = 0;
int test_alloc_base::converted = 0;

#endif // TEST_LIBCXX_NEW

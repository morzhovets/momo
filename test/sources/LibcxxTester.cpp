/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxTester.cpp

\**********************************************************/

#include "pch.h"

#include "LibcxxTester.h"

int DefaultOnly::count = 0;

int Counter_base::gConstructed = 0;

size_t malloc_allocator_base::outstanding_bytes = 0;
size_t malloc_allocator_base::alloc_count = 0;
size_t malloc_allocator_base::dealloc_count = 0;
bool malloc_allocator_base::disable_default_constructor = false;

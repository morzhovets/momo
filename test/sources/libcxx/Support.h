/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/libcxx/Support.h

\**********************************************************/

#pragma once

#include <cfloat>
#include <string>
#include <cmath>

//#define LIBCPP_HAS_BAD_NEWS_FOR_MOMO

#define TEST_STD_VER 20

#include "support/test_macros.h"

#include "support/MoveOnly.h"
#include "support/Copyable.h"
#include "support/NotConstructible.h"
#include "support/DefaultOnly.h"
#include "support/Emplaceable.h"
#include "support/Counter.h"
#include "support/test_allocator.h"
#include "support/test_iterators.h"
#include "support/test_compare.h"
#include "support/test_hash.h"
#include "support/private_constructor.h"
#include "support/is_transparent.h"
#include "support/test_transparent_unordered.h"
#include "support/test_comparisons.h"
#include "support/emplace_constructible.h"
#include "support/allocators.h"
#include "support/container_test_types.h"
#include "support/deduction_guides_sfinae_checks.h"
#include "support/test_container_comparisons.h"
#include "support/check_consecutive.h"
#include "support/set_allocator_requirement_test_templates.h"
#include "support/map_allocator_requirement_test_templates.h"
#include "support/asan_testing.h"
#include "support/min_allocator.h"

#define TEST_LIBCPP_ASSERT_FAILURE(expr, mess) try { (void)(expr); assert(false); } catch (...) {}

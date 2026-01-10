/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/libcxx20/Support.h

\**********************************************************/

#pragma once

#include <cfloat>
#include <string>
#include <cmath>
#include <optional>
#include <sstream>

static_assert(TEST_LIBCXX_VERSION >= 20);

//#define LIBCPP_HAS_BAD_NEWS_FOR_MOMO

#if defined(__cpp_lib_containers_ranges)
# define TEST_STD_VER 23
#else
# define TEST_STD_VER 20
#endif

#ifdef MOMO_DISABLE_EXCEPTIONS
# define TEST_HAS_NO_EXCEPTIONS
#endif

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
#include "support/counting_predicates.h"
#include "support/asan_testing.h"
#include "support/increasing_allocator.h"
#include "support/min_allocator.h"

#if TEST_STD_VER >= 23
# include "support/insert_range_sequence_containers.h"
# include "support/from_range_sequence_containers.h"
# include "support/insert_range_maps_sets.h"
# include "support/from_range_associative_containers.h"
# include "support/from_range_unordered_containers.h"
#else
namespace libcxx_insert_range_seq {}
namespace libcxx_from_range_seq {}
namespace libcxx_insert_range_maps_sets {}
namespace libcxx_from_range_assoc {}
namespace libcxx_from_range_unord {}
#endif

#define TEST_LIBCPP_ASSERT_FAILURE(expr, mess) try { (void)(expr); assert(false); } catch (...) {}

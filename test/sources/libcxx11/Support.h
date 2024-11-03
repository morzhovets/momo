/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/libcxx11/Support.h

\**********************************************************/

#pragma once

#include <cfloat>
#include <string>
#include <cmath>

#ifdef __cpp_lib_optional
# include <optional>
#endif

#if !defined(TEST_MSVC) && !defined(__cpp_lib_transparent_operators)
# define LIBCPP_HAS_NO_TRANSPARENT_OPERATORS
#endif

#if !defined(LIBCPP_TEST_DEDUCTION_GUIDES) && defined(MOMO_HAS_DEDUCTION_GUIDES)
# if defined(TEST_MSVC) && _MSC_VER >= 1924
#  define LIBCPP_TEST_DEDUCTION_GUIDES
# endif
# if defined(TEST_GCC) && __GNUC__ >= 8
#  define LIBCPP_TEST_DEDUCTION_GUIDES
# endif
#endif

//#define LIBCPP_HAS_BAD_NEWS_FOR_MOMO

//#define LIBCPP_TEST_MIN_ALLOCATOR
//#define LIBCPP_TEST_STACK_ALLOCATOR

#include "support/MoveOnly.h"
#include "support/Copyable.h"
#include "support/NotConstructible.h"
#include "support/DefaultOnly.h"
#include "support/Emplaceable.h"
#include "support/Counter.h"
#include "support/Moveable.h"
#include "support/test_allocator.h"
#include "support/test_iterators.h"
#include "support/test_compare.h"
#include "support/test_hash.h"
#include "support/private_constructor.h"
#include "support/is_transparent.h"
#include "support/test_macros.h"
#include "support/test_transparent_unordered.h"
//#include "support/min_allocator.h"
//#include "support/stack_allocator.h"

namespace libcxx_insert_range_seq {}
namespace libcxx_from_range_seq {}
namespace libcxx_insert_range_maps_sets {}
namespace libcxx_from_range_assoc {}
namespace libcxx_from_range_unord {}

struct LibcppIntHash
{
	size_t operator()(int key) const noexcept
	{
		return static_cast<size_t>(key);
	}
};

#define LIBCPP_CATCH(expr) try { (void)(expr); assert(false); } catch (...) {}

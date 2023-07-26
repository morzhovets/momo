/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxTester.h

\**********************************************************/

#pragma once

#include <iostream>
#include <cfloat>
#include <string>
#include <cmath>

#ifdef __cpp_lib_optional
#include <optional>
#endif

#if !defined(TEST_MSVC) && !defined(__cpp_lib_transparent_operators)
#define LIBCPP_HAS_NO_TRANSPARENT_OPERATORS
#endif

#if !defined(LIBCPP_TEST_DEDUCTION_GUIDES) && defined(MOMO_HAS_DEDUCTION_GUIDES)
#if defined(TEST_MSVC) && _MSC_VER >= 1924
#define LIBCPP_TEST_DEDUCTION_GUIDES
#endif
#if defined(TEST_GCC) && __GNUC__ >= 8
#define LIBCPP_TEST_DEDUCTION_GUIDES
#endif
#endif

//#define LIBCPP_HAS_BAD_NEWS_FOR_MOMO
//#define LIBCPP_TEST_MIN_ALLOCATOR

#include "libcxx/support/MoveOnly.h"
#include "libcxx/support/Copyable.h"
#include "libcxx/support/NotConstructible.h"
#include "libcxx/support/DefaultOnly.h"
#include "libcxx/support/Emplaceable.h"
#include "libcxx/support/Counter.h"
#include "libcxx/support/Moveable.h"
#include "libcxx/support/stack_allocator.h"
#include "libcxx/support/test_allocator.h"
#include "libcxx/support/test_iterators.h"
#include "libcxx/support/test_compare.h"
#include "libcxx/support/test_hash.h"
#include "libcxx/support/private_constructor.h"
#include "libcxx/support/is_transparent.h"
#include "libcxx/support/test_macros.h"
#include "libcxx/support/test_transparent_unordered.h"
//#include "libcxx/support/min_allocator.h"

struct LibcppIntHash
{
	size_t operator()(int key) const noexcept
	{
		return static_cast<size_t>(key);
	}
};

#define LIBCPP_CATCH(expr) try { (void)(expr); assert(false); } catch (...) {}

#define LIBCXX_TEST_BEGIN(name) \
	namespace name { \
		void main(); \
		static int testLibcxx = [] \
		{ \
			std::cout << LIBCXX_TEST_PREFIX << "_" << #name << ": " << std::flush; \
			main(); \
			std::cout << "ok" << std::endl; \
			return 0; \
		}();

#define LIBCXX_TEST_END }

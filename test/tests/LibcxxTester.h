/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxTester.h

\**********************************************************/

#pragma once

#include <iostream>
#include <cfloat>
#include <string>
#include <cmath>

#ifdef __cpp_lib_optional
#include <optional>
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
	typedef int argument_type;

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

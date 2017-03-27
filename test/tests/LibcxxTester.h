/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/LibcxxTester.h

\**********************************************************/

#pragma once

#include <iostream>
#include <cfloat>
#include <string>
#include <cmath>

#ifdef _MSC_VER

#if _MSC_VER >= 1900
#define _LIBCPP_STD_VER 14
#else
#define _LIBCPP_STD_VER 11
#define _LIBCPP_HAS_NO_GENERALIZED_INITIALIZERS
#endif

#else

#if __cplusplus >= 201402L
#define _LIBCPP_STD_VER 14
#else
#define _LIBCPP_STD_VER 11
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
//#include "libcxx/support/min_allocator.h"

struct LibcppIntHash
{
	typedef int argument_type;

	size_t operator()(int key) const MOMO_NOEXCEPT
	{
		return (size_t)key;
	}
};

#define LIBCPP_CATCH(expr) try { (void)(expr); assert(false); } catch (...) {}

#define LIBCXX_TEST_BEGIN(name) \
	namespace name { \
	void main(); \
	int TestLibcxx() \
	{ \
		std::cout << LIBCXX_TEST_PREFIX << "_" << #name << ": " << std::flush; \
		main(); \
		std::cout << "ok" << std::endl; \
		return 0; \
	} \
	static int testLibcxx = (TestLibcxx(), 0);

#define LIBCXX_TEST_END }

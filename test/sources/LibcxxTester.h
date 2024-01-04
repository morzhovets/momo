/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxTester.h

\**********************************************************/

#pragma once

#define LIBCXX_TO_STR(expr) #expr
#define LIBCXX_HEADER(header) LIBCXX_TO_STR(libcxx/header)

#include LIBCXX_HEADER(Support.h)

#include <iostream>

#define LIBCXX_NAMESPACE_STD_BEGIN

#define LIBCXX_NAMESPACE_STD_END

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

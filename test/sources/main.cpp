/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/main.cpp

\**********************************************************/

#include "pch.h"

#include "../../include/momo/UserSettings.h"

#include "TestSettings.h"

#ifdef TEST_LIBCXX_VERSION
# include "LibcxxTester.h"
#endif

int main()
{
	std::cout << std::endl;

	std::cout << "__cplusplus: " << __cplusplus << std::endl;

	std::cout << "sizeof(void*): " << sizeof(void*) << std::endl;

	std::cout << "_MSC_VER: ";
#ifdef _MSC_VER
	std::cout << _MSC_VER;
#endif
	std::cout << std::endl;

	std::cout << "__GNUC__: ";
#ifdef __GNUC__
	std::cout << __GNUC__;
#endif
	std::cout << std::endl;

	std::cout << "__clang_major__: ";
#ifdef __clang_major__
	std::cout << __clang_major__;
#endif
	std::cout << std::endl;

	std::cout << "_MSVC_STL_VERSION: ";
#ifdef _MSVC_STL_VERSION
	std::cout << _MSVC_STL_VERSION;
#endif
	std::cout << std::endl;

	std::cout << "__GLIBCXX__: ";
#ifdef __GLIBCXX__
	std::cout << __GLIBCXX__;
#endif
	std::cout << std::endl;

	std::cout << "_LIBCPP_VERSION: ";
#ifdef _LIBCPP_VERSION
	std::cout << _LIBCPP_VERSION;
#endif
	std::cout << std::endl;

	std::cout << "_M_CEE: ";
#ifdef _M_CEE
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "MOMO_MAX_ALIGNMENT: ";
#ifdef MOMO_MAX_ALIGNMENT
	std::cout << MOMO_MAX_ALIGNMENT;
#endif
	std::cout << std::endl;

	std::cout << "MOMO_USE_SSE2: ";
#ifdef MOMO_USE_SSE2
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "MOMO_LITTLE_ENDIAN: ";
#ifdef MOMO_LITTLE_ENDIAN
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "MOMO_CTZ64: ";
#ifdef MOMO_CTZ64
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "MOMO_CONSTEXPR_VERSION: " << MOMO_CONSTEXPR_VERSION << std::endl;

	std::cout << "MOMO_HAS_DEDUCTION_GUIDES: ";
#ifdef MOMO_HAS_DEDUCTION_GUIDES
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "MOMO_HAS_GUARANTEED_COPY_ELISION: ";
#ifdef MOMO_HAS_GUARANTEED_COPY_ELISION
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "MOMO_HAS_THREE_WAY_COMPARISON: ";
#ifdef MOMO_HAS_THREE_WAY_COMPARISON
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "MOMO_HAS_CONTAINERS_RANGES: ";
#ifdef MOMO_HAS_CONTAINERS_RANGES
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "MOMO_USE_MEM_MANAGER_WIN: ";
#ifdef MOMO_USE_MEM_MANAGER_WIN
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "MOMO_DISABLE_TYPE_INFO: ";
#ifdef MOMO_DISABLE_TYPE_INFO
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "NDEBUG: ";
#ifdef NDEBUG
	std::cout << "defined";
#endif
	std::cout << std::endl;

#ifdef TEST_LIBCXX_VERSION

	std::cout << "TEST_LIBCXX_VERSION: " << TEST_LIBCXX_VERSION << std::endl;

#if TEST_LIBCXX_VERSION >= 20

	std::cout << "TEST_STD_VER: " << TEST_STD_VER << std::endl;

#else

	std::cout << "LIBCPP_HAS_NO_TRANSPARENT_OPERATORS: ";
#ifdef LIBCPP_HAS_NO_TRANSPARENT_OPERATORS
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "LIBCPP_TEST_DEDUCTION_GUIDES: ";
#ifdef LIBCPP_TEST_DEDUCTION_GUIDES
	std::cout << "defined";
#endif
	std::cout << std::endl;

#endif

#endif // TEST_LIBCXX_VERSION

	return 0;
}

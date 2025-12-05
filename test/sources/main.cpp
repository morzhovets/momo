/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
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

	std::cout << "MOMO_DISABLE_EXCEPTIONS: ";
#ifdef MOMO_DISABLE_EXCEPTIONS
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "MOMO_DISABLE_TYPE_INFO: ";
#ifdef MOMO_DISABLE_TYPE_INFO
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "MOMO_MAX_ALIGNMENT: ";
#ifdef MOMO_MAX_ALIGNMENT
	std::cout << MOMO_MAX_ALIGNMENT;
#endif
	std::cout << std::endl;

	std::cout << "MOMO_USE_HASH_TRAITS_STRING_SPECIALIZATION: ";
#ifdef MOMO_USE_HASH_TRAITS_STRING_SPECIALIZATION
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "MOMO_USE_DEFAULT_MEM_MANAGER_IN_STD: ";
#ifdef MOMO_USE_DEFAULT_MEM_MANAGER_IN_STD
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "MOMO_USE_MEM_MANAGER_WIN: ";
#ifdef MOMO_USE_MEM_MANAGER_WIN
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "MOMO_USE_SSE2: ";
#ifdef MOMO_USE_SSE2
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "MOMO_CATCH_ALL: ";
#ifdef MOMO_CATCH_ALL
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "MOMO_TEST_NO_EXCEPTIONS_RTTI: ";
#ifdef MOMO_TEST_NO_EXCEPTIONS_RTTI
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "MOMO_TEST_EXTRA_SETTINGS: ";
#ifdef MOMO_TEST_EXTRA_SETTINGS
	std::cout << "defined";
#endif
	std::cout << std::endl;

	std::cout << "NDEBUG: ";
#ifdef NDEBUG
	std::cout << "defined";
#endif
	std::cout << std::endl;

#ifdef TEST_LIBCXX_VERSION
	std::cout << "TEST_STD_VER: " << TEST_STD_VER << std::endl;
#endif

	return 0;
}

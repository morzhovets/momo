/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/main.cpp

\**********************************************************/

#include "pch.h"

int main()
{
#ifdef _MSC_VER
	std::cout << "_MSC_VER: " << _MSC_VER << std::endl;
#endif
#ifdef __GNUC__
	std::cout << "__GNUC__: " << __GNUC__ << std::endl;
#endif
#ifdef __clang_major__
	std::cout << "__clang_major__: " << __clang_major__ << std::endl;
#endif
	std::cout << "__cplusplus: " << __cplusplus << std::endl;
	std::cout << "sizeof(void*): " << sizeof(void*) << std::endl;
	return 0;
}

// momo_clang_lib.cpp : Defines the exported functions for the static library.
//

#include "momo_clang_lib.h"

// This is an example of an exported variable
int nmomo_clang_lib=0;

// This is an example of an exported function.
int fnmomo_clang_lib(void)
{
    return 42;
}

// This is the constructor of a class that has been exported.
// see momo_clang_lib.h for the class definition
Cmomo_clang_lib::Cmomo_clang_lib()
{
    return;
}

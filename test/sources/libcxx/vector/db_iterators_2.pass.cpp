//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Modified for https://github.com/morzhovets/momo project.
//
//===----------------------------------------------------------------------===//

// <vector>

// Compare iterators from different containers with <.

#if _LIBCPP_DEBUG >= 1

//#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))

//#include <vector>
//#include <cassert>
//#include <iterator>
//#include <exception>
//#include <cstdlib>

//#include "min_allocator.h"

void main()
{
    {
    typedef int T;
    typedef vector<T> C;
    C c1;
    C c2;
    LIBCPP_CATCH(c1.begin() < c2.begin());
    //assert(false);
    }
#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef int T;
    typedef vector<T, min_allocator<T>> C;
    C c1;
    C c2;
    bool b = c1.begin() < c2.begin();
    assert(false);
    }
#endif
#endif
}

#else

void main()
{
}

#endif
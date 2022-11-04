//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// Add to iterator out of bounds.

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
    C c(1);
    C::iterator i = c.begin();
    i += 1;
    assert(i == c.end());
    i = c.begin();
    LIBCPP_CATCH(i += 2);
    //assert(false);
    }
#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef int T;
    typedef vector<T, min_allocator<T>> C;
    C c(1);
    C::iterator i = c.begin();
    i += 1;
    assert(i == c.end());
    i = c.begin();
    i += 2;
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

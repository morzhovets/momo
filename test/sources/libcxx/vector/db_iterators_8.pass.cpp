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

// Dereference non-dereferenceable iterator.

#ifdef LIBCXX_TEST_FAILURE

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
    C::iterator i = c.end();
    LIBCPP_CATCH(*i);
    //assert(false);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef int T;
    typedef vector<T, min_allocator<T>> C;
    C c(1);
    C::iterator i = c.end();
    T j = *i;
    assert(false);
    }
#endif
}

#else

void main()
{
}

#endif

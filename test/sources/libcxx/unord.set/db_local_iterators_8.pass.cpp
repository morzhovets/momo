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

// <unordered_set>

// Dereference non-dereferenceable iterator.

#ifdef LIBCXX_TEST_FAILURE

//#include <unordered_set>
//#include <cassert>
//#include <iterator>
//#include <exception>
//#include <cstdlib>

//#include "min_allocator.h"

void main()
{
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    {
    typedef int T;
    typedef unordered_set<T> C;
    C c(1);
    C::local_iterator i = c.end(0);
    LIBCPP_CATCH(*i);
    //assert(false);
    }
#endif
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef int T;
    typedef unordered_set<T, min_allocator<T>> C;
    C c(1);
    C::local_iterator i = c.end(0);
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

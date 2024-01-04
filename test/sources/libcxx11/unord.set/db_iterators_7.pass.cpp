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

// Increment iterator past end.

#ifdef LIBCXX_TEST_FAILURE

//#include <unordered_set>
//#include <cassert>
//#include <iterator>
//#include <exception>
//#include <cstdlib>

//#include "min_allocator.h"

void main()
{
    {
    typedef int T;
    typedef unordered_set<T> C;
    C c(1);
    C::iterator i = c.begin();
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    ++i;
#else
    LIBCPP_CATCH(++i);
#endif
    assert(i == c.end());
    LIBCPP_CATCH(++i);
    //assert(false);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef int T;
    typedef unordered_set<T, min_allocator<T>> C;
    C c(1);
    C::iterator i = c.begin();
    ++i;
    assert(i == c.end());
    ++i;
    assert(false);
    }
#endif
}

#else

void main()
{
}

#endif

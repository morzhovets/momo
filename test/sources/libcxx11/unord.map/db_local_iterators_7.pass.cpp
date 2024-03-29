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

// <unordered_map>

// Increment local_iterator past end.

#ifdef LIBCXX_TEST_FAILURE

//#include <unordered_map>
//#include <string>
//#include <cassert>
//#include <iterator>
//#include <exception>
//#include <cstdlib>

//#include "min_allocator.h"

void main()
{
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    {
    typedef unordered_map<int, std::string> C;
    C c(1);
    C::local_iterator i = c.begin(0);
    ++i;
    LIBCPP_CATCH(++i);
    //assert(false);
    }
#endif
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
                        min_allocator<std::pair<const int, std::string>>> C;
    C c(1);
    C::local_iterator i = c.begin(0);
    ++i;
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

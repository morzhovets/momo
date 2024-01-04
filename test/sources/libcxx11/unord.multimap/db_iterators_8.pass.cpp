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

// Dereference non-dereferenceable iterator.

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
    {
    typedef unordered_multimap<int, std::string> C;
    C c;
    c.insert(std::make_pair(1, "one"));
    C::iterator i = c.end();
    LIBCPP_CATCH(*i);
    //assert(false);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
                        min_allocator<std::pair<const int, std::string>>> C;
    C c;
    c.insert(std::make_pair(1, "one"));
    C::iterator i = c.end();
    C::value_type j = *i;
    assert(false);
    }
#endif
}

#else

void main()
{
}

#endif

//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <set>

// class set

// set();

//#include <set>
//#include <cassert>

//#include "min_allocator.h"

void main()
{
    {
    set<int> m;
    assert(m.empty());
    assert(m.begin() == m.end());
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    set<int, std::less<int>, min_allocator<int>> m;
    assert(m.empty());
    assert(m.begin() == m.end());
    }
#endif
    {
    set<int> m = {};
    assert(m.empty());
    assert(m.begin() == m.end());
    }
//#endif
}

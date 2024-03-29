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

// <map>

// class map

// map();

//#include <map>
//#include <cassert>

//#include "min_allocator.h"

void main()
{
    {
    map<int, double> m;
    assert(m.empty());
    assert(m.begin() == m.end());
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    map<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> m;
    assert(m.empty());
    assert(m.begin() == m.end());
    }
#endif
    {
    map<int, double> m = {};
    assert(m.empty());
    assert(m.begin() == m.end());
    }
//#endif
}

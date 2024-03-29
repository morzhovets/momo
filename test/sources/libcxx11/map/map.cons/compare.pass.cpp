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

// explicit map(const key_compare& comp);

//#include <map>
//#include <cassert>

//#include "../../../test_compare.h"
//#include "min_allocator.h"

void main()
{
    {
    typedef test_compare<std::less<int> > C;
    map<int, double, C> m(C(3));
    assert(m.empty());
    assert(m.begin() == m.end());
    assert(m.key_comp() == C(3));
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef test_compare<std::less<int> > C;
    map<int, double, C, min_allocator<std::pair<const int, double>>> m(C(3));
    assert(m.empty());
    assert(m.begin() == m.end());
    assert(m.key_comp() == C(3));
    }
#endif
}

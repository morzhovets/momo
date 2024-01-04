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

// explicit map(const allocator_type& a);

//#include <map>
//#include <cassert>

//#include "test_allocator.h"
//#include "min_allocator.h"

void main()
{
    {
    typedef std::less<int> C;
    typedef test_allocator<std::pair<const int, double> > A;
    map<int, double, C, A> m(A(5));
    assert(m.empty());
    assert(m.begin() == m.end());
    assert(m.get_allocator() == A(5));
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef std::less<int> C;
    typedef min_allocator<std::pair<const int, double> > A;
    map<int, double, C, A> m(A{});
    assert(m.empty());
    assert(m.begin() == m.end());
    assert(m.get_allocator() == A());
    }
#endif
}

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

// UNSUPPORTED: c++98, c++03

// <set>

// class multiset

// multiset& operator=(initializer_list<value_type> il);

//#include <set>
//#include <cassert>

//#include "min_allocator.h"

void main()
{
    {
    typedef multiset<int> C;
    typedef C::value_type V;
    C m = {10, 8};
    m = {1, 2, 3, 4, 5, 6};
    assert(m.size() == 6);
    assert(distance(m.begin(), m.end()) == 6);
    C::const_iterator i = m.cbegin();
    assert(*i == V(1));
    assert(*++i == V(2));
    assert(*++i == V(3));
    assert(*++i == V(4));
    assert(*++i == V(5));
    assert(*++i == V(6));
    }
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef multiset<int, std::less<int>, min_allocator<int>> C;
    typedef C::value_type V;
    C m = {10, 8};
    m = {1, 2, 3, 4, 5, 6};
    assert(m.size() == 6);
    assert(distance(m.begin(), m.end()) == 6);
    C::const_iterator i = m.cbegin();
    assert(*i == V(1));
    assert(*++i == V(2));
    assert(*++i == V(3));
    assert(*++i == V(4));
    assert(*++i == V(5));
    assert(*++i == V(6));
    }
#endif
}

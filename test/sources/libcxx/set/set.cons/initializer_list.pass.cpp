//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Modified for https://github.com/morzhovets/momo project.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++03

// <set>

// class set

// set(initializer_list<value_type> il, const key_compare& comp = key_compare());

void main()
{
    {
    typedef set<int> C;
    typedef C::value_type V;
    C m = {1, 2, 3, 4, 5, 6};
    assert(m.size() == 6);
    assert(std::distance(m.begin(), m.end()) == 6);
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
    typedef set<int, std::less<int>, min_allocator<int>> C;
    typedef C::value_type V;
    C m = {1, 2, 3, 4, 5, 6};
    assert(m.size() == 6);
    assert(std::distance(m.begin(), m.end()) == 6);
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

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

// void insert(initializer_list<value_type> il);

void main()
{
    {
    typedef set<int> C;
    typedef C::value_type V;
    C m = {10, 8};
    m.insert({1, 2, 3, 4, 5, 6});
    assert(m.size() == 8);
    assert(static_cast<std::size_t>(std::distance(m.begin(), m.end())) == m.size());
    C::const_iterator i = m.cbegin();
    assert(*i == V(1));
    assert(*++i == V(2));
    assert(*++i == V(3));
    assert(*++i == V(4));
    assert(*++i == V(5));
    assert(*++i == V(6));
    assert(*++i == V(8));
    assert(*++i == V(10));
    }
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef set<int, std::less<int>, min_allocator<int>> C;
    typedef C::value_type V;
    C m = {10, 8};
    m.insert({1, 2, 3, 4, 5, 6});
    assert(m.size() == 8);
    assert(static_cast<std::size_t>(std::distance(m.begin(), m.end())) == m.size());
    C::const_iterator i = m.cbegin();
    assert(*i == V(1));
    assert(*++i == V(2));
    assert(*++i == V(3));
    assert(*++i == V(4));
    assert(*++i == V(5));
    assert(*++i == V(6));
    assert(*++i == V(8));
    assert(*++i == V(10));
    }
#endif
}

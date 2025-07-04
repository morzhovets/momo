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

// <unordered_map>

// template <class Key, class T, class Hash = hash<Key>, class Pred = equal_to<Key>,
//           class Alloc = allocator<pair<const Key, T>>>
// class unordered_map

// iterator insert(const_iterator p, const value_type& x);

template<class Container>
void do_insert_hint_const_lvalue_test()
{
    typedef Container C;
    typedef typename C::iterator R;
    typedef typename C::value_type VT;
    C c;
    typename C::const_iterator e = /*c.end()*/c.find(3.5);
    const VT v1(3.5, 3);
    R r = c.insert(e, v1);
    assert(c.size() == 1);
    assert(r->first == 3.5);
    assert(r->second == 3);

#ifndef LIBCXX_TEST_HINT_ITERATORS
    const VT v2(3.5, 4);
    r = c.insert(/*r*/c.find(3.5), v2);
    assert(c.size() == 1);
    assert(r->first == 3.5);
    assert(r->second == 3);
#endif

    const VT v3(4.5, 4);
    r = c.insert(/*c.end()*/c.find(4.5), v3);
    assert(c.size() == 2);
    assert(r->first == 4.5);
    assert(r->second == 4);

    const VT v4(5.5, 4);
    r = c.insert(/*c.end()*/c.find(5.5), v4);
    assert(c.size() == 3);
    assert(r->first == 5.5);
    assert(r->second == 4);
}

int main(int, char**)
{
    do_insert_hint_const_lvalue_test<std::unordered_map<double, int> >();
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<double, int, std::hash<double>, std::equal_to<double>,
                            min_allocator<std::pair<const double, int>>> C;

        do_insert_hint_const_lvalue_test<C>();
    }
#endif

    return 0;
}

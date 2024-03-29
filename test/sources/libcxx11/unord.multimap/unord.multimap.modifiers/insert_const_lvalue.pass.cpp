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

// template <class Key, class T, class Hash = hash<Key>, class Pred = equal_to<Key>,
//           class Alloc = allocator<pair<const Key, T>>>
// class unordered_multimap

// iterator insert(const value_type& x);

//#include <unordered_map>
//#include <cassert>

//#include "min_allocator.h"

void main()
{
    {
        typedef unordered_multimap<double, int> C;
        typedef C::iterator R;
        typedef C::value_type P;
        C c;
        P p33(3.5, 3);
        R r = c.insert(p33);
        assert(c.size() == 1);
        assert(r->first == 3.5);
        assert(r->second == 3);

        P p34(3.5, 4);
        r = c.insert(p34);
        assert(c.size() == 2);
        assert(r->first == 3.5);
        assert(r->second == 4);

        P p44(4.5, 4);
        r = c.insert(p44);
        assert(c.size() == 3);
        assert(r->first == 4.5);
        assert(r->second == 4);

        P p54(5.5, 4);
        r = c.insert(p54);
        assert(c.size() == 4);
        assert(r->first == 5.5);
        assert(r->second == 4);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef unordered_multimap<double, int, std::hash<double>, std::equal_to<double>,
                            min_allocator<std::pair<const double, int>>> C;
        typedef C::iterator R;
        typedef C::value_type P;
        C c;
        R r = c.insert(P(3.5, 3));
        assert(c.size() == 1);
        assert(r->first == 3.5);
        assert(r->second == 3);

        r = c.insert(P(3.5, 4));
        assert(c.size() == 2);
        assert(r->first == 3.5);
        assert(r->second == 4);

        r = c.insert(P(4.5, 4));
        assert(c.size() == 3);
        assert(r->first == 4.5);
        assert(r->second == 4);

        r = c.insert(P(5.5, 4));
        assert(c.size() == 4);
        assert(r->first == 5.5);
        assert(r->second == 4);
    }
#endif
}

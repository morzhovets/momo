//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_map>

// template <class Key, class T, class Hash = hash<Key>, class Pred = equal_to<Key>,
//           class Alloc = allocator<pair<const Key, T>>>
// class unordered_multimap

// template <class P,
//           class = typename enable_if<is_convertible<P, value_type>::value>::type>
//     iterator insert(P&& x);

//#include <unordered_map>
//#include <cassert>

//#include "MoveOnly.h"
//#include "min_allocator.h"

void main()
{
    {
        typedef unordered_multimap<double, int> C;
        typedef C::iterator R;
        typedef std::pair<double, short> P;
        C c;
        R r = c.insert(P(3.5, short{3}));
        assert(c.size() == 1);
        assert(r->first == 3.5);
        assert(r->second == 3);

        r = c.insert(P(3.5, short{4}));
        assert(c.size() == 2);
        assert(r->first == 3.5);
        assert(r->second == 4);

        r = c.insert(P(4.5, short{4}));
        assert(c.size() == 3);
        assert(r->first == 4.5);
        assert(r->second == 4);

        r = c.insert(P(5.5, short{4}));
        assert(c.size() == 4);
        assert(r->first == 5.5);
        assert(r->second == 4);
    }
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    {
        typedef unordered_multimap<MoveOnly, MoveOnly> C;
        typedef C::iterator R;
        typedef std::pair<MoveOnly, MoveOnly> P;
        C c;
        R r = c.insert(P(3, 3));
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == 3);

        r = c.insert(P(3, 4));
        assert(c.size() == 2);
        assert(r->first == 3);
        assert(r->second == 4);

        r = c.insert(P(4, 4));
        assert(c.size() == 3);
        assert(r->first == 4);
        assert(r->second == 4);

        r = c.insert(P(5, 4));
        assert(c.size() == 4);
        assert(r->first == 5);
        assert(r->second == 4);
    }
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef unordered_multimap<double, int, std::hash<double>, std::equal_to<double>,
                            min_allocator<std::pair<const double, int>>> C;
        typedef C::iterator R;
        typedef std::pair<double, short> P;
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
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    {
        typedef unordered_multimap<MoveOnly, MoveOnly, std::hash<MoveOnly>, std::equal_to<MoveOnly>,
                            min_allocator<std::pair<const MoveOnly, MoveOnly>>> C;
        typedef C::iterator R;
        typedef std::pair<MoveOnly, MoveOnly> P;
        C c;
        R r = c.insert(P(3, 3));
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == 3);

        r = c.insert(P(3, 4));
        assert(c.size() == 2);
        assert(r->first == 3);
        assert(r->second == 4);

        r = c.insert(P(4, 4));
        assert(c.size() == 3);
        assert(r->first == 4);
        assert(r->second == 4);

        r = c.insert(P(5, 4));
        assert(c.size() == 4);
        assert(r->first == 5);
        assert(r->second == 4);
    }
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
#endif
}

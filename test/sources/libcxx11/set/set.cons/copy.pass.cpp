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

// <set>

// class set

// set(const set& m);

//#include <set>
//#include <cassert>

//#include "../../../test_compare.h"
//#include "test_allocator.h"

void main()
{
    {
        typedef int V;
        V ar[] =
        {
            1,
            1,
            1,
            2,
            2,
            2,
            3,
            3,
            3
        };
        typedef test_compare<std::less<int> > C;
        typedef test_allocator<V> A;
        set<int, C, A> mo(ar, ar+sizeof(ar)/sizeof(ar[0]), C(5), A(7));
        set<int, C, A> m = mo;
        assert(m.get_allocator() == A(7));
        assert(m.key_comp() == C(5));
        assert(m.size() == 3);
        assert(std::distance(m.begin(), m.end()) == 3);
        assert(*m.begin() == 1);
        assert(*std::next(m.begin()) == 2);
        assert(*std::next(m.begin(), 2) == 3);

        assert(mo.get_allocator() == A(7));
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 3);
        assert(std::distance(mo.begin(), mo.end()) == 3);
        assert(*mo.begin() == 1);
        assert(*std::next(mo.begin()) == 2);
        assert(*std::next(mo.begin(), 2) == 3);
    }
#ifndef _LIBCPP_HAS_NO_ADVANCED_SFINAE
    {
        typedef int V;
        V ar[] =
        {
            1,
            1,
            1,
            2,
            2,
            2,
            3,
            3,
            3
        };
        typedef test_compare<std::less<int> > C;
        typedef other_allocator<V> A;
        set<int, C, A> mo(ar, ar+sizeof(ar)/sizeof(ar[0]), C(5), A(7));
        set<int, C, A> m = mo;
        assert(m.get_allocator() == A(-2));
        assert(m.key_comp() == C(5));
        assert(m.size() == 3);
        assert(std::distance(m.begin(), m.end()) == 3);
        assert(*m.begin() == 1);
        assert(*std::next(m.begin()) == 2);
        assert(*std::next(m.begin(), 2) == 3);

        assert(mo.get_allocator() == A(7));
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 3);
        assert(std::distance(mo.begin(), mo.end()) == 3);
        assert(*mo.begin() == 1);
        assert(*std::next(mo.begin()) == 2);
        assert(*std::next(mo.begin(), 2) == 3);
    }
#endif  // _LIBCPP_HAS_NO_ADVANCED_SFINAE
}

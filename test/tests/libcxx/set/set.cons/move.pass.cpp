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

// set(set&& s);

//#include <set>
//#include <cassert>

//#include "../../../test_compare.h"
//#include "test_allocator.h"
//#include "min_allocator.h"

void main()
{
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    {
        typedef int V;
        typedef test_compare<std::less<int> > C;
        typedef test_allocator<V> A;
        set<int, C, A> mo(C(5), A(7));
        set<int, C, A> m = std::move(mo);
        assert(m.get_allocator() == A(7));
        assert(m.key_comp() == C(5));
        assert(m.size() == 0);
        assert(std::distance(m.begin(), m.end()) == 0);

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(mo.get_allocator() == A(7));
        assert(mo.key_comp() == C(5));
#endif
        assert(mo.size() == 0);
        assert(std::distance(mo.begin(), mo.end()) == 0);
    }
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
        set<int, C, A> m = std::move(mo);
        assert(m.get_allocator() == A(7));
        assert(m.key_comp() == C(5));
        assert(m.size() == 3);
        assert(std::distance(m.begin(), m.end()) == 3);
        assert(*m.begin() == 1);
        assert(*std::next(m.begin()) == 2);
        assert(*std::next(m.begin(), 2) == 3);

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(mo.get_allocator() == A(7));
        assert(mo.key_comp() == C(5));
#endif
        assert(mo.size() == 0);
        assert(std::distance(mo.begin(), mo.end()) == 0);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
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
        typedef min_allocator<V> A;
        set<int, C, A> mo(ar, ar+sizeof(ar)/sizeof(ar[0]), C(5), A());
        set<int, C, A> m = std::move(mo);
        assert(m.get_allocator() == A());
        assert(m.key_comp() == C(5));
        assert(m.size() == 3);
        assert(std::distance(m.begin(), m.end()) == 3);
        assert(*m.begin() == 1);
        assert(*std::next(m.begin()) == 2);
        assert(*std::next(m.begin(), 2) == 3);

        assert(mo.get_allocator() == A());
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(std::distance(mo.begin(), mo.end()) == 0);
    }
#endif
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
}

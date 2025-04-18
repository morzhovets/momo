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

// <map>

// class multimap

// multimap(multimap&& m);

//#include <map>
//#include <cassert>

//#include "../../../test_compare.h"
//#include "test_allocator.h"
//#include "min_allocator.h"

void main()
{
    typedef std::pair<const int, double> V;
    {
        typedef test_compare<std::less<int> > C;
        typedef test_allocator<V> A;
        multimap<int, double, C, A> mo(C(5), A(7));
        multimap<int, double, C, A> m = std::move(mo);
        assert(m.get_allocator() == A(7));
        assert(m.key_comp() == C(5));
        assert(m.size() == 0);
        assert(std::distance(m.begin(), m.end()) == 0);

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(mo.get_allocator() == A(test_alloc_base::moved_value));
#endif
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(std::distance(mo.begin(), mo.end()) == 0);
    }
    {
        V ar[] =
        {
            V(1, 1),
            V(1, 1.5),
            V(1, 2),
            V(2, 1),
            V(2, 1.5),
            V(2, 2),
            V(3, 1),
            V(3, 1.5),
            V(3, 2),
        };
        typedef test_compare<std::less<int> > C;
        typedef test_allocator<V> A;
        multimap<int, double, C, A> mo(ar, ar+sizeof(ar)/sizeof(ar[0]), C(5), A(7));
        multimap<int, double, C, A> m = std::move(mo);
        assert(m.get_allocator() == A(7));
        assert(m.key_comp() == C(5));
        assert(m.size() == 9);
        assert(std::distance(m.begin(), m.end()) == 9);
        assert(*m.begin() == V(1, 1));
        assert(*std::next(m.begin()) == V(1, 1.5));
        assert(*std::next(m.begin(), 2) == V(1, 2));
        assert(*std::next(m.begin(), 3) == V(2, 1));
        assert(*std::next(m.begin(), 4) == V(2, 1.5));
        assert(*std::next(m.begin(), 5) == V(2, 2));
        assert(*std::next(m.begin(), 6) == V(3, 1));
        assert(*std::next(m.begin(), 7) == V(3, 1.5));
        assert(*std::next(m.begin(), 8) == V(3, 2));

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(mo.get_allocator() == A(test_alloc_base::moved_value));
#endif
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(std::distance(mo.begin(), mo.end()) == 0);
    }
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef test_compare<std::less<int> > C;
        typedef min_allocator<V> A;
        multimap<int, double, C, A> mo(C(5), A());
        multimap<int, double, C, A> m = std::move(mo);
        assert(m.get_allocator() == A());
        assert(m.key_comp() == C(5));
        assert(m.size() == 0);
        assert(std::distance(m.begin(), m.end()) == 0);

        assert(mo.get_allocator() == A());
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(std::distance(mo.begin(), mo.end()) == 0);
    }
    {
        V ar[] =
        {
            V(1, 1),
            V(1, 1.5),
            V(1, 2),
            V(2, 1),
            V(2, 1.5),
            V(2, 2),
            V(3, 1),
            V(3, 1.5),
            V(3, 2),
        };
        typedef test_compare<std::less<int> > C;
        typedef min_allocator<V> A;
        multimap<int, double, C, A> mo(ar, ar+sizeof(ar)/sizeof(ar[0]), C(5), A());
        multimap<int, double, C, A> m = std::move(mo);
        assert(m.get_allocator() == A());
        assert(m.key_comp() == C(5));
        assert(m.size() == 9);
        assert(std::distance(m.begin(), m.end()) == 9);
        assert(*m.begin() == V(1, 1));
        assert(*std::next(m.begin()) == V(1, 1.5));
        assert(*std::next(m.begin(), 2) == V(1, 2));
        assert(*std::next(m.begin(), 3) == V(2, 1));
        assert(*std::next(m.begin(), 4) == V(2, 1.5));
        assert(*std::next(m.begin(), 5) == V(2, 2));
        assert(*std::next(m.begin(), 6) == V(3, 1));
        assert(*std::next(m.begin(), 7) == V(3, 1.5));
        assert(*std::next(m.begin(), 8) == V(3, 2));

        assert(mo.get_allocator() == A());
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(std::distance(mo.begin(), mo.end()) == 0);
    }
#endif
}

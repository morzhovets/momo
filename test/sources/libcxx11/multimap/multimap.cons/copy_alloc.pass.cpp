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

// class multimap

// multimap(const multimap& m, const allocator_type& a);

//#include <map>
//#include <cassert>

//#include "../../../test_compare.h"
//#include "test_allocator.h"
//#include "min_allocator.h"

void main()
{
    {
    typedef std::pair<const int, double> V;
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
    multimap<int, double, C, A> m(mo, A(3));
    assert(m == mo);
    assert(m.get_allocator() == A(3));
    assert(m.key_comp() == C(5));

    assert(mo.get_allocator() == A(7));
    assert(mo.key_comp() == C(5));
    }
//#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef std::pair<const int, double> V;
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
    multimap<int, double, C, A> m(mo, A());
    assert(m == mo);
    assert(m.get_allocator() == A());
    assert(m.key_comp() == C(5));

    assert(mo.get_allocator() == A());
    assert(mo.key_comp() == C(5));
    }
    {
    typedef std::pair<const int, double> V;
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
    typedef explicit_allocator<V> A;
    multimap<int, double, C, A> mo(ar, ar+sizeof(ar)/sizeof(ar[0]), C(5), A{});
    multimap<int, double, C, A> m(mo, A{});
    assert(m == mo);
    assert(m.get_allocator() == A{});
    assert(m.key_comp() == C(5));

    assert(mo.get_allocator() == A{});
    assert(mo.key_comp() == C(5));
    }
#endif
}

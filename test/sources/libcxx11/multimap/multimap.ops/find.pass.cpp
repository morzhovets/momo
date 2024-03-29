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

//       iterator find(const key_type& k);
// const_iterator find(const key_type& k) const;

//#include <map>
//#include <cassert>

//#include "test_macros.h"
//#include "min_allocator.h"
//#include "private_constructor.hpp"
//#include "is_transparent.h"

void main()
{
    typedef std::pair<const int, double> V;
    {
    typedef multimap<int, double> M;
    {
        typedef M::iterator R;
        V ar[] =
        {
            V(5, 1),
            V(5, 2),
            V(5, 3),
            V(7, 1),
            V(7, 2),
            V(7, 3),
            V(9, 1),
            V(9, 2),
            V(9, 3)
        };
        M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        R r = m.find(5);
        assert(r == m.begin());
        r = m.find(6);
        assert(r == m.end());
        r = m.find(7);
        assert(r == next(m.begin(), 3));
        r = m.find(8);
        assert(r == m.end());
        r = m.find(9);
        assert(r == next(m.begin(), 6));
        r = m.find(10);
        assert(r == m.end());
    }
    {
        typedef M::const_iterator R;
        V ar[] =
        {
            V(5, 1),
            V(5, 2),
            V(5, 3),
            V(7, 1),
            V(7, 2),
            V(7, 3),
            V(9, 1),
            V(9, 2),
            V(9, 3)
        };
        const M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        R r = m.find(5);
        assert(r == m.begin());
        r = m.find(6);
        assert(r == m.end());
        r = m.find(7);
        assert(r == next(m.begin(), 3));
        r = m.find(8);
        assert(r == m.end());
        r = m.find(9);
        assert(r == next(m.begin(), 6));
        r = m.find(10);
        assert(r == m.end());
    }
    }
//#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef multimap<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
    {
        typedef M::iterator R;
        V ar[] =
        {
            V(5, 1),
            V(5, 2),
            V(5, 3),
            V(7, 1),
            V(7, 2),
            V(7, 3),
            V(9, 1),
            V(9, 2),
            V(9, 3)
        };
        M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        R r = m.find(5);
        assert(r == m.begin());
        r = m.find(6);
        assert(r == m.end());
        r = m.find(7);
        assert(r == next(m.begin(), 3));
        r = m.find(8);
        assert(r == m.end());
        r = m.find(9);
        assert(r == next(m.begin(), 6));
        r = m.find(10);
        assert(r == m.end());
    }
    {
        typedef M::const_iterator R;
        V ar[] =
        {
            V(5, 1),
            V(5, 2),
            V(5, 3),
            V(7, 1),
            V(7, 2),
            V(7, 3),
            V(9, 1),
            V(9, 2),
            V(9, 3)
        };
        const M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        R r = m.find(5);
        assert(r == m.begin());
        r = m.find(6);
        assert(r == m.end());
        r = m.find(7);
        assert(r == next(m.begin(), 3));
        r = m.find(8);
        assert(r == m.end());
        r = m.find(9);
        assert(r == next(m.begin(), 6));
        r = m.find(10);
        assert(r == m.end());
    }
    }
#endif
//#if _LIBCPP_STD_VER > 11
#ifndef LIBCPP_HAS_NO_TRANSPARENT_OPERATORS
    {
    typedef multimap<int, double, std::less<>> M;
    typedef M::iterator R;

        V ar[] =
        {
            V(5, 1),
            V(5, 2),
            V(5, 3),
            V(7, 1),
            V(7, 2),
            V(7, 3),
            V(9, 1),
            V(9, 2),
            V(9, 3)
        };
        M m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        R r = m.find(5);
        assert(r == m.begin());
        r = m.find(6);
        assert(r == m.end());
        r = m.find(7);
        assert(r == next(m.begin(), 3));
        r = m.find(8);
        assert(r == m.end());
        r = m.find(9);
        assert(r == next(m.begin(), 6));
        r = m.find(10);
        assert(r == m.end());

        r = m.find(C2Int(5));
        assert(r == m.begin());
        r = m.find(C2Int(6));
        assert(r == m.end());
        r = m.find(C2Int(7));
        assert(r == next(m.begin(), 3));
        r = m.find(C2Int(8));
        assert(r == m.end());
        r = m.find(C2Int(9));
        assert(r == next(m.begin(), 6));
        r = m.find(C2Int(10));
        assert(r == m.end());
    }

    {
    typedef PrivateConstructor PC;
    typedef multimap<PC, double, std::less<>> M;
    typedef M::iterator R;

    M m;
    m.insert ( std::make_pair<PC, double> ( PC::make(5), 1 ));
    m.insert ( std::make_pair<PC, double> ( PC::make(5), 2 ));
    m.insert ( std::make_pair<PC, double> ( PC::make(5), 3 ));
    m.insert ( std::make_pair<PC, double> ( PC::make(7), 1 ));
    m.insert ( std::make_pair<PC, double> ( PC::make(7), 2 ));
    m.insert ( std::make_pair<PC, double> ( PC::make(7), 3 ));
    m.insert ( std::make_pair<PC, double> ( PC::make(9), 1 ));
    m.insert ( std::make_pair<PC, double> ( PC::make(9), 2 ));
    m.insert ( std::make_pair<PC, double> ( PC::make(9), 3 ));

    R r = m.find(5);
    assert(r == m.begin());
    r = m.find(6);
    assert(r == m.end());
    r = m.find(7);
    assert(r == next(m.begin(), 3));
    r = m.find(8);
    assert(r == m.end());
    r = m.find(9);
    assert(r == next(m.begin(), 6));
    r = m.find(10);
    assert(r == m.end());
    }
#endif
}

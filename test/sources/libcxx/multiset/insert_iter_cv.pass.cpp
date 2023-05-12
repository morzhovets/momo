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

// class multiset

// iterator insert(const_iterator position, const value_type& v);

//#include <set>
//#include <cassert>

//#include "min_allocator.h"

void main()
{
    {
        typedef multiset<int> M;
        typedef M::iterator R;
        M m;

        int v2 = 2;
        R r = m.insert(m.cend(), v2);
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*r == 2);

        int v1 = 1;
        r = m.insert(m.cend(), v1);
        assert(r == m.begin());
        assert(m.size() == 2);
        assert(*r == 1);

        int v3 = 3;
        r = m.insert(m.cend(), v3);
        assert(r == prev(m.end()));
        assert(m.size() == 3);
        assert(*r == 3);

        r = m.insert(m.cend(), v3);
        assert(r == prev(m.end()));
        assert(m.size() == 4);
        assert(*r == 3);
    }
//#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef multiset<int, std::less<int>, min_allocator<int>> M;
        typedef M::iterator R;
        M m;
        R r = m.insert(m.cend(), M::value_type(2));
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*r == 2);

        r = m.insert(m.cend(), M::value_type(1));
        assert(r == m.begin());
        assert(m.size() == 2);
        assert(*r == 1);

        r = m.insert(m.cend(), M::value_type(3));
        assert(r == prev(m.end()));
        assert(m.size() == 3);
        assert(*r == 3);

        r = m.insert(m.cend(), M::value_type(3));
        assert(r == prev(m.end()));
        assert(m.size() == 4);
        assert(*r == 3);
    }
#endif
}

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

// <set>

// class multiset

// template <class... Args>
//   iterator emplace_hint(const_iterator position, Args&&... args);

//#include <set>
//#include <cassert>

//#include "../../Emplaceable.h"
//#include "DefaultOnly.h"
//#include "min_allocator.h"

void main()
{
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    {
        typedef multiset<DefaultOnly> M;
        typedef M::iterator R;
        M m;
        assert(DefaultOnly::count == 0);
        R r = m.emplace_hint(m.cend());
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*m.begin() == DefaultOnly());
        assert(DefaultOnly::count == 1);

        r = m.emplace_hint(m.cbegin());
        assert(r == m.begin());
        assert(m.size() == 2);
        assert(*m.begin() == DefaultOnly());
        assert(DefaultOnly::count == 2);
    }
    assert(DefaultOnly::count == 0);
#endif
    {
        typedef multiset<Emplaceable> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace_hint(m.cend());
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*m.begin() == Emplaceable());
        r = m.emplace_hint(m.cend(), 2, 3.5);
        assert(r == next(m.begin()));
        assert(m.size() == 2);
        assert(*r == Emplaceable(2, 3.5));
        r = m.emplace_hint(m.cbegin(), 2, 3.5);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(r == next(m.begin()));
#else
        assert(r == next(next(m.begin())));
#endif
        assert(m.size() == 3);
        assert(*r == Emplaceable(2, 3.5));
    }
    {
        typedef multiset<int> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace_hint(m.cend(), M::value_type(2));
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*r == 2);
        r = m.emplace_hint(m.cend(), M::value_type(1));
        assert(r == m.begin());
        assert(m.size() == 2);
        assert(*r == 1);
    }
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef multiset<int, std::less<int>, min_allocator<int>> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace_hint(m.cend(), M::value_type(2));
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(*r == 2);
    }
#endif
}

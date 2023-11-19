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

// template <class... Args>
//   pair<iterator, bool> emplace(Args&&... args);

void main()
{
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    {
        typedef set<DefaultOnly> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        assert(DefaultOnly::count == 0);
        R r = m.emplace();
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(*m.begin() == DefaultOnly());
        assert(DefaultOnly::count == 1);

        r = m.emplace();
        assert(!r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(*m.begin() == DefaultOnly());
        assert(DefaultOnly::count == 1);
    }
    assert(DefaultOnly::count == 0);
#endif
    {
        typedef set<Emplaceable> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.emplace();
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(*m.begin() == Emplaceable());
        r = m.emplace(2, 3.5);
        assert(r.second);
        assert(r.first == next(m.begin()));
        assert(m.size() == 2);
        assert(*r.first == Emplaceable(2, 3.5));
        r = m.emplace(2, 3.5);
        assert(!r.second);
        assert(r.first == next(m.begin()));
        assert(m.size() == 2);
        assert(*r.first == Emplaceable(2, 3.5));
    }
    {
        typedef set<int> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.emplace(M::value_type(2));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(*r.first == 2);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef set<int, std::less<int>, min_allocator<int>> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.emplace(M::value_type(2));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(*r.first == 2);
    }
#endif
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
}

//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// class map

// template <class... Args>
//   pair<iterator, bool> emplace(Args&&... args);

//#include <map>
//#include <cassert>
//#include <tuple>

//#include "../../../Emplaceable.h"
//#include "DefaultOnly.h"
//#include "min_allocator.h"

void main()
{
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    {
        typedef map<int, DefaultOnly> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        assert(DefaultOnly::count == 0);
        R r = m.emplace();
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 0);
        assert(m.begin()->second == DefaultOnly());
        assert(DefaultOnly::count == 1);
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
                                                std::forward_as_tuple());
        assert(r.second);
        assert(r.first == next(m.begin()));
        assert(m.size() == 2);
        assert(next(m.begin())->first == 1);
        assert(next(m.begin())->second == DefaultOnly());
        assert(DefaultOnly::count == 2);
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
                                                std::forward_as_tuple());
        assert(!r.second);
        assert(r.first == next(m.begin()));
        assert(m.size() == 2);
        assert(next(m.begin())->first == 1);
        assert(next(m.begin())->second == DefaultOnly());
        assert(DefaultOnly::count == 2);
    }
    assert(DefaultOnly::count == 0);
#endif
    {
        typedef map<int, Emplaceable> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.emplace(std::piecewise_construct, std::forward_as_tuple(2),
                                                  std::forward_as_tuple());
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == Emplaceable());
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
                                                std::forward_as_tuple(2, 3.5));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == Emplaceable(2, 3.5));
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
                                                std::forward_as_tuple(2, 3.5));
        assert(!r.second);
        assert(r.first == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == Emplaceable(2, 3.5));
    }
    {
        typedef map<int, double> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.emplace(M::value_type(2, 3.5));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 3.5);

        r = m.emplace(1, 3.5);
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == 3.5);

        r = m.emplace();
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 3);
        assert(m.begin()->first == 0);
        assert(m.begin()->second == 0.0);
    }
    {
        typedef map<std::string, double> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.emplace(std::piecewise_construct, std::forward_as_tuple(2, 'a'),
                                                  std::forward_as_tuple(3.5));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == "aa");
        assert(m.begin()->second == 3.5);

        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(2, 'a'),
                                                std::forward_as_tuple(2.5));
        assert(!r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == "aa");
        assert(m.begin()->second == 3.5);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef map<int, DefaultOnly, std::less<int>, min_allocator<std::pair<const int, DefaultOnly>>> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        assert(DefaultOnly::count == 0);
        R r = m.emplace();
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 0);
        assert(m.begin()->second == DefaultOnly());
        assert(DefaultOnly::count == 1);
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
                                                std::forward_as_tuple());
        assert(r.second);
        assert(r.first == next(m.begin()));
        assert(m.size() == 2);
        assert(next(m.begin())->first == 1);
        assert(next(m.begin())->second == DefaultOnly());
        assert(DefaultOnly::count == 2);
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
                                                std::forward_as_tuple());
        assert(!r.second);
        assert(r.first == next(m.begin()));
        assert(m.size() == 2);
        assert(next(m.begin())->first == 1);
        assert(next(m.begin())->second == DefaultOnly());
        assert(DefaultOnly::count == 2);
    }
    assert(DefaultOnly::count == 0);
    {
        typedef map<int, Emplaceable, std::less<int>, min_allocator<std::pair<const int, Emplaceable>>> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.emplace(std::piecewise_construct, std::forward_as_tuple(2),
                                                  std::forward_as_tuple());
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == Emplaceable());
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
                                                std::forward_as_tuple(2, 3.5));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == Emplaceable(2, 3.5));
        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(1),
                                                std::forward_as_tuple(2, 3.5));
        assert(!r.second);
        assert(r.first == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == Emplaceable(2, 3.5));
    }
    {
        typedef map<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.emplace(M::value_type(2, 3.5));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 3.5);
    }
#endif
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
}

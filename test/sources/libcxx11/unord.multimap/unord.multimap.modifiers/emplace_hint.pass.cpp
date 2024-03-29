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

// <unordered_map>

// template <class Key, class T, class Hash = hash<Key>, class Pred = equal_to<Key>,
//           class Alloc = allocator<pair<const Key, T>>>
// class unordered_multimap

// template <class... Args>
//     iterator emplace_hint(const_iterator p, Args&&... args);

//#include <unordered_map>
//#include <cassert>

//#include "../../../Emplaceable.h"
//#include "min_allocator.h"

void main()
{
#ifndef _LIBCPP_HAS_NO_VARIADICS
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    {
        typedef unordered_multimap<int, Emplaceable> C;
        typedef C::iterator R;
        C c;
        C::const_iterator e = c.end();
        R r = c.emplace_hint(e, std::piecewise_construct, std::forward_as_tuple(3),
                                                          std::forward_as_tuple());
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == Emplaceable());

        r = c.emplace_hint(c.end(), std::pair<const int, Emplaceable>(3, Emplaceable(5, 6)));
        assert(c.size() == 2);
        assert(r->first == 3);
        assert(r->second == Emplaceable(5, 6));
        assert(r == next(c.begin()));

        r = c.emplace_hint(r, 3, Emplaceable(6, 7));
        assert(c.size() == 3);
        assert(r->first == 3);
        assert(r->second == Emplaceable(6, 7));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(r == next(c.begin()));
#endif
        r = c.begin();
        assert(r->first == 3);
        assert(r->second == Emplaceable());
        r = next(r, 2);
        assert(r->first == 3);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(r->second == Emplaceable(5, 6));
#endif

        r = c.emplace_hint(c.end());
        assert(c.size() == 4);
        assert(r->first == 0);
        assert(r->second == Emplaceable());
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef unordered_multimap<int, Emplaceable, std::hash<int>, std::equal_to<int>,
                            min_allocator<std::pair<const int, Emplaceable>>> C;
        typedef C::iterator R;
        C c;
        C::const_iterator e = c.end();
        R r = c.emplace_hint(e, std::piecewise_construct, std::forward_as_tuple(3),
                                                          std::forward_as_tuple());
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == Emplaceable());

        r = c.emplace_hint(c.end(), std::pair<const int, Emplaceable>(3, Emplaceable(5, 6)));
        assert(c.size() == 2);
        assert(r->first == 3);
        assert(r->second == Emplaceable(5, 6));
        assert(r == next(c.begin()));

        r = c.emplace_hint(r, std::piecewise_construct, std::forward_as_tuple(3),
                                                        std::forward_as_tuple(6, 7));
        assert(c.size() == 3);
        assert(r->first == 3);
        assert(r->second == Emplaceable(6, 7));
        assert(r == next(c.begin()));
        r = c.begin();
        assert(r->first == 3);
        assert(r->second == Emplaceable());
        r = next(r, 2);
        assert(r->first == 3);
        assert(r->second == Emplaceable(5, 6));
    }
#endif
#ifdef LIBCXX_TEST_FAILURE
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    {
        typedef unordered_multimap<int, Emplaceable> C;
        typedef C::iterator R;
        typedef C::value_type P;
        C c;
        C c2;
        R r = c.emplace_hint(c2.end(), std::piecewise_construct,
                                       std::forward_as_tuple(3),
                                       std::forward_as_tuple());
        assert(false);
    }
#endif
#endif
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
#endif  // _LIBCPP_HAS_NO_VARIADICS
}

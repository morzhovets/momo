//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_map>

// template <class Key, class T, class Hash = hash<Key>, class Pred = equal_to<Key>,
//           class Alloc = allocator<pair<const Key, T>>>
// class unordered_map

// template <class... Args>
//     iterator emplace_hint(const_iterator p, Args&&... args);

#if _LIBCPP_DEBUG >= 1
//#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))
#endif

//#include <unordered_map>
//#include <cassert>

//#include "../../../Emplaceable.h"
//#include "min_allocator.h"

void main()
{
#ifndef _LIBCPP_HAS_NO_VARIADICS
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    {
        typedef unordered_map<int, Emplaceable> C;
        typedef C::iterator R;
        C c;
        C::const_iterator e = /*c.end()*/c.find(3);
        R r = c.emplace_hint(e, std::piecewise_construct, std::forward_as_tuple(3),
                                                          std::forward_as_tuple());
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == Emplaceable());

        r = c.emplace_hint(/*c.end()*/c.find(4), std::pair<const int, Emplaceable>(4, Emplaceable(5, 6)));
        assert(c.size() == 2);
        assert(r->first == 4);
        assert(r->second == Emplaceable(5, 6));

        r = c.emplace_hint(/*c.end()*/c.find(5), 5, Emplaceable(6, 7));
        assert(c.size() == 3);
        assert(r->first == 5);
        assert(r->second == Emplaceable(6, 7));

        r = c.emplace_hint(c.find(0));
        assert(c.size() == 4);
        assert(r->first == 0);
        assert(r->second == Emplaceable());
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef unordered_map<int, Emplaceable, std::hash<int>, std::equal_to<int>,
                            min_allocator<std::pair<const int, Emplaceable>>> C;
        typedef C::iterator R;
        C c;
        C::const_iterator e = c.end();
        R r = c.emplace_hint(e, std::piecewise_construct, std::forward_as_tuple(3),
                                                          std::forward_as_tuple());
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == Emplaceable());

        r = c.emplace_hint(c.end(), std::pair<const int, Emplaceable>(4, Emplaceable(5, 6)));
        assert(c.size() == 2);
        assert(r->first == 4);
        assert(r->second == Emplaceable(5, 6));

        r = c.emplace_hint(c.end(), std::piecewise_construct, std::forward_as_tuple(5),
                                                       std::forward_as_tuple(6, 7));
        assert(c.size() == 3);
        assert(r->first == 5);
        assert(r->second == Emplaceable(6, 7));
    }
#endif
#if _LIBCPP_DEBUG >= 1
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    {
        typedef unordered_map<int, Emplaceable> C;
        typedef C::iterator R;
        typedef C::value_type P;
        C c;
        C c2;
        LIBCPP_CATCH(c.emplace_hint(c2.end(), std::piecewise_construct,
                                       std::forward_as_tuple(3),
                                       std::forward_as_tuple()));
        //assert(false);
    }
#endif
#endif
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
#endif  // _LIBCPP_HAS_NO_VARIADICS
}

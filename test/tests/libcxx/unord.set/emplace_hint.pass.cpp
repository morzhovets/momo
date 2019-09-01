//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_set>

// template <class Value, class Hash = hash<Value>, class Pred = equal_to<Value>,
//           class Alloc = allocator<Value>>
// class unordered_set

// template <class... Args>
//     iterator emplace_hint(const_iterator p, Args&&... args);

#if _LIBCPP_DEBUG >= 1
//#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))
#endif

//#include <unordered_set>
//#include <cassert>

//#include "../../Emplaceable.h"
//#include "min_allocator.h"

void main()
{
#ifndef _LIBCPP_HAS_NO_VARIADICS
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    {
        typedef unordered_set<Emplaceable> C;
        typedef C::iterator R;
        C c;
        //C::const_iterator e = c.end();
        R r = c.emplace_hint(/*e*/c.find(Emplaceable()));
        assert(c.size() == 1);
        assert(*r == Emplaceable());

        r = c.emplace_hint(/*e*/c.find(Emplaceable(5, 6)), Emplaceable(5, 6));
        assert(c.size() == 2);
        assert(*r == Emplaceable(5, 6));

#ifndef MOMO_USE_UNORDERED_HINT_ITERATORS
        r = c.emplace_hint(r, 5, 6);
        assert(c.size() == 2);
        assert(*r == Emplaceable(5, 6));
#endif
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef unordered_set<Emplaceable, std::hash<Emplaceable>,
                      std::equal_to<Emplaceable>, min_allocator<Emplaceable>> C;
        typedef C::iterator R;
        C c;
        C::const_iterator e = c.end();
        R r = c.emplace_hint(e);
        assert(c.size() == 1);
        assert(*r == Emplaceable());

        r = c.emplace_hint(e, Emplaceable(5, 6));
        assert(c.size() == 2);
        assert(*r == Emplaceable(5, 6));

        r = c.emplace_hint(r, 5, 6);
        assert(c.size() == 2);
        assert(*r == Emplaceable(5, 6));
    }
#endif
#if _LIBCPP_DEBUG >= 1
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    {
        typedef unordered_set<Emplaceable> C;
        typedef C::iterator R;
        C c1;
        C c2;
        LIBCPP_CATCH(c1.emplace_hint(c2.begin(), 5, 6));
        //assert(false);
    }
#endif
#endif
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
#endif  // _LIBCPP_HAS_NO_VARIADICS
}

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

// iterator insert(const_iterator p, const value_type& x);

#if _LIBCPP_DEBUG >= 1
//#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))
#endif

//#include <unordered_set>
//#include <cassert>

//#include "min_allocator.h"

void main()
{
    {
        typedef unordered_set<double> C;
        typedef C::iterator R;
        typedef C::value_type P;
        C c;

        //C::const_iterator e = c.end();
        P p3 = 3.5;
        R r = c.insert(/*e*/c.find(3.5), p3);
        assert(c.size() == 1);
        assert(*r == 3.5);

#ifndef MOMO_USE_UNORDERED_HINT_ITERATORS
        r = c.insert(/*e*/c.find(3.5), p3);
        assert(c.size() == 1);
        assert(*r == 3.5);
#endif

        P p4 = 4.5;
        r = c.insert(/*e*/c.find(4.5), p4);
        assert(c.size() == 2);
        assert(*r == 4.5);

        P p5 = 5.5;
        r = c.insert(/*e*/c.find(5.5), p5);
        assert(c.size() == 3);
        assert(*r == 5.5);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef unordered_set<double, std::hash<double>,
                                std::equal_to<double>, min_allocator<double>> C;
        typedef C::iterator R;
        typedef C::value_type P;
        C c;
        C::const_iterator e = c.end();
        R r = c.insert(e, P(3.5));
        assert(c.size() == 1);
        assert(*r == 3.5);

        r = c.insert(e, P(3.5));
        assert(c.size() == 1);
        assert(*r == 3.5);

        r = c.insert(e, P(4.5));
        assert(c.size() == 2);
        assert(*r == 4.5);

        r = c.insert(e, P(5.5));
        assert(c.size() == 3);
        assert(*r == 5.5);
    }
#endif
#if _LIBCPP_DEBUG >= 1
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    {
        typedef unordered_set<double> C;
        typedef C::iterator R;
        typedef C::value_type P;
        C c;
        C c2;
        C::const_iterator e = c2.end();
        P v(3.5);
        LIBCPP_CATCH(c.insert(e, v));
        //assert(false);
    }
#endif
#endif
}

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

// size_type bucket(const key_type& __k) const;

#ifdef _LIBCPP_DEBUG
//#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))
#endif

//#include <unordered_set>
//#include <cassert>

//#include "min_allocator.h"

void main()
{
    {
        typedef unordered_set<int, LibcppIntHash> C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        const C c(std::begin(a), std::end(a));
        size_t bc = c.bucket_count();
        assert(bc >= 5);
        for (size_t i = 0; i < 13; ++i)
            assert(c.bucket((int)i) == i % bc);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef unordered_set<int, std::hash<int>, std::equal_to<int>, min_allocator<int>> C;
        typedef int P;
        P a[] =
        {
            P(1),
            P(2),
            P(3),
            P(4),
            P(1),
            P(2)
        };
        const C c(std::begin(a), std::end(a));
        size_t bc = c.bucket_count();
        assert(bc >= 5);
        for (size_t i = 0; i < 13; ++i)
            assert(c.bucket(i) == i % bc);
    }
#endif
#if _LIBCPP_DEBUG_LEVEL >= 1
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    {
        typedef unordered_set<int> C;
        C c;
        LIBCPP_CATCH(c.bucket(3));
        //assert(false);
    }
#endif
#endif
}

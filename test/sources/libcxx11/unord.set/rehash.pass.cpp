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

// <unordered_set>

// template <class Value, class Hash = hash<Value>, class Pred = equal_to<Value>,
//           class Alloc = allocator<Value>>
// class unordered_set

// void rehash(size_type n);

//#include <unordered_set>
//#include <cassert>

//#include "min_allocator.h"

template <class C>
void test(const C& c)
{
    assert(c.size() == 4);
    assert(c.count(1) == 1);
    assert(c.count(2) == 1);
    assert(c.count(3) == 1);
    assert(c.count(4) == 1);
}

void main()
{
    {
        typedef unordered_set<int> C;
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
        C c(a, a + sizeof(a)/sizeof(a[0]));
        test(c);
        assert(c.bucket_count() >= 5);
        c.rehash(3);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c.bucket_count() == 5);
#endif
        test(c);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        c.max_load_factor(2);
#else
        c.max_load_factor(0.5);
#endif
        c.rehash(3);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c.bucket_count() == 3);
#endif
        test(c);
        c.rehash(31);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c.bucket_count() == 31);
#else
        assert(c.bucket_count() >= 31);
#endif
        test(c);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef unordered_set<int, std::hash<int>,
                                      std::equal_to<int>, min_allocator<int>> C;
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
        C c(a, a + sizeof(a)/sizeof(a[0]));
        test(c);
        assert(c.bucket_count() >= 5);
        c.rehash(3);
        assert(c.bucket_count() == 5);
        test(c);
        c.max_load_factor(2);
        c.rehash(3);
        assert(c.bucket_count() == 3);
        test(c);
        c.rehash(31);
        assert(c.bucket_count() == 31);
        test(c);
    }
#endif
}

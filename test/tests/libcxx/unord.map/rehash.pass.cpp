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

// void rehash(size_type n);

//#include <unordered_map>
//#include <string>
//#include <cassert>

//#include "min_allocator.h"

template <class C>
void test(const C& c)
{
    assert(c.size() == 4);
    assert(c.at(1) == "one");
    assert(c.at(2) == "two");
    assert(c.at(3) == "three");
    assert(c.at(4) == "four");
}

void main()
{
    {
        typedef unordered_map<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
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
        typedef unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
                            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
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

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

// unordered_map(const unordered_map& u);

//#include <unordered_map>
//#include <string>
//#include <cassert>
//#include <cfloat>

//#include "../../../test_compare.h"
//#include "../../../test_hash.h"
//#include "test_allocator.h"
//#include "min_allocator.h"

void main()
{
    {
        typedef unordered_map<int, std::string,
                                   test_hash<std::hash<int> >,
                                   test_compare<std::equal_to<int> >,
                                   test_allocator<std::pair<const int, std::string> >
                                   > C;
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
        C c0(a, a + sizeof(a)/sizeof(a[0]),
            7,
            test_hash<std::hash<int> >(8),
            test_compare<std::equal_to<int> >(9),
            test_allocator<std::pair<const int, std::string> >(10)
           );
        C c = c0;
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c.bucket_count() == 7);
#else
        assert(c.bucket_count() >= 7);
#endif
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<std::hash<int> >(8));
        assert(c.key_eq() == test_compare<std::equal_to<int> >(9));
        assert(c.get_allocator() ==
               (test_allocator<std::pair<const int, std::string> >(10)));
        assert(!c.empty());
        assert((size_t)std::distance(c.begin(), c.end()) == c.size());
        assert((size_t)std::distance(c.cbegin(), c.cend()) == c.size());
        assert(fabs(c.load_factor() - (float)c.size()/c.bucket_count()) < FLT_EPSILON);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c.max_load_factor() == 1);
#endif
    }
#ifndef _LIBCPP_HAS_NO_ADVANCED_SFINAE
    {
        typedef unordered_map<int, std::string,
                                   test_hash<std::hash<int> >,
                                   test_compare<std::equal_to<int> >,
                                   other_allocator<std::pair<const int, std::string> >
                                   > C;
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
        C c0(a, a + sizeof(a)/sizeof(a[0]),
            7,
            test_hash<std::hash<int> >(8),
            test_compare<std::equal_to<int> >(9),
            other_allocator<std::pair<const int, std::string> >(10)
           );
        C c = c0;
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c.bucket_count() == 7);
#else
        assert(c.bucket_count() >= 7);
#endif
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<std::hash<int> >(8));
        assert(c.key_eq() == test_compare<std::equal_to<int> >(9));
        assert(c.get_allocator() ==
               (other_allocator<std::pair<const int, std::string> >(-2)));
        assert(!c.empty());
        assert((size_t)std::distance(c.begin(), c.end()) == c.size());
        assert((size_t)std::distance(c.cbegin(), c.cend()) == c.size());
        assert(fabs(c.load_factor() - (float)c.size()/c.bucket_count()) < FLT_EPSILON);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c.max_load_factor() == 1);
#endif
    }
#endif  // _LIBCPP_HAS_NO_ADVANCED_SFINAE
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef unordered_map<int, std::string,
                                   test_hash<std::hash<int> >,
                                   test_compare<std::equal_to<int> >,
                                   min_allocator<std::pair<const int, std::string> >
                                   > C;
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
        C c0(a, a + sizeof(a)/sizeof(a[0]),
            7,
            test_hash<std::hash<int> >(8),
            test_compare<std::equal_to<int> >(9),
            min_allocator<std::pair<const int, std::string> >()
           );
        C c = c0;
        assert(c.bucket_count() == 7);
        assert(c.size() == 4);
        assert(c.at(1) == "one");
        assert(c.at(2) == "two");
        assert(c.at(3) == "three");
        assert(c.at(4) == "four");
        assert(c.hash_function() == test_hash<std::hash<int> >(8));
        assert(c.key_eq() == test_compare<std::equal_to<int> >(9));
        assert(c.get_allocator() ==
               (min_allocator<std::pair<const int, std::string> >()));
        assert(!c.empty());
        assert((size_t)std::distance(c.begin(), c.end()) == c.size());
        assert((size_t)std::distance(c.cbegin(), c.cend()) == c.size());
        assert(fabs(c.load_factor() - (float)c.size()/c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
#endif
}

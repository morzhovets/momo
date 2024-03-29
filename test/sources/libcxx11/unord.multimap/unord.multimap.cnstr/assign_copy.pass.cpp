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

// unordered_multimap& operator=(const unordered_multimap& u);

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
        typedef test_allocator<std::pair<const int, std::string> > A;
        typedef unordered_multimap<int, std::string,
                                   test_hash<LibcppIntHash>, //test_hash<std::hash<int> >,
                                   test_compare<std::equal_to<int> >,
                                   A
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
            test_hash<LibcppIntHash>(8),
            test_compare<std::equal_to<int> >(9),
            A(10)
           );
        C c(a, a + 2,
            7,
            test_hash<LibcppIntHash>(2),
            test_compare<std::equal_to<int> >(3),
            A(4)
           );
        c = c0;
        ///assert(c.bucket_count() == 7);
        assert(c.size() == 6);
        C::const_iterator i = c.cbegin();
        assert(i->first == 1);
        assert(i->second == "one");
        ++i;
        assert(i->first == 1);
        assert(i->second == "four");
        ++i;
        assert(i->first == 2);
        assert(i->second == "two");
        ++i;
        assert(i->first == 2);
        assert(i->second == "four");
        ++i;
        assert(i->first == 3);
        assert(i->second == "three");
        ++i;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(c.hash_function() == test_hash<LibcppIntHash>(8));
        assert(c.key_eq() == test_compare<std::equal_to<int> >(9));
        assert(c.get_allocator() == A(4));
        assert(!c.empty());
        assert(momo::internal::UIntMath<>::Dist(c.begin(), c.end()) == c.size());
        assert(momo::internal::UIntMath<>::Dist(c.cbegin(), c.cend()) == c.size());
        ///assert(fabs(c.load_factor() - static_cast<float>(c.size())/c.bucket_count()) < FLT_EPSILON);
        ///assert(c.max_load_factor() == 1);
    }
    {
        typedef unordered_multimap<int, std::string> C;
        typedef std::pair<const int, std::string> P;
        const P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a+sizeof(a)/sizeof(a[0]));
        C *p = &c;
        c = *p;
        assert(c.size() == 6);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(std::is_permutation(c.begin(), c.end(), a));
#endif
    }
    {
        typedef other_allocator<std::pair<const int, std::string> > A;
        typedef unordered_multimap<int, std::string,
                                   test_hash<LibcppIntHash>, //test_hash<std::hash<int> >,
                                   test_compare<std::equal_to<int> >,
                                   A
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
            test_hash<LibcppIntHash>(8),
            test_compare<std::equal_to<int> >(9),
            A(10)
           );
        C c(a, a + 2,
            7,
            test_hash<LibcppIntHash>(2),
            test_compare<std::equal_to<int> >(3),
            A(4)
           );
        c = c0;
        ///assert(c.bucket_count() >= 7);
        assert(c.size() == 6);
        C::const_iterator i = c.cbegin();
        assert(i->first == 1);
        assert(i->second == "one");
        ++i;
        assert(i->first == 1);
        assert(i->second == "four");
        ++i;
        assert(i->first == 2);
        assert(i->second == "two");
        ++i;
        assert(i->first == 2);
        assert(i->second == "four");
        ++i;
        assert(i->first == 3);
        assert(i->second == "three");
        ++i;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(c.hash_function() == test_hash<LibcppIntHash>(8));
        assert(c.key_eq() == test_compare<std::equal_to<int> >(9));
        assert(c.get_allocator() == A(10));
        assert(!c.empty());
        assert(momo::internal::UIntMath<>::Dist(c.begin(), c.end()) == c.size());
        assert(momo::internal::UIntMath<>::Dist(c.cbegin(), c.cend()) == c.size());
        ///assert(fabs(c.load_factor() - static_cast<float>(c.size())/c.bucket_count()) < FLT_EPSILON);
        ///assert(c.max_load_factor() == 1);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef min_allocator<std::pair<const int, std::string> > A;
        typedef unordered_multimap<int, std::string,
                                   test_hash<std::hash<int> >,
                                   test_compare<std::equal_to<int> >,
                                   A
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
            A()
           );
        C c(a, a + 2,
            7,
            test_hash<std::hash<int> >(2),
            test_compare<std::equal_to<int> >(3),
            A()
           );
        c = c0;
        assert(c.bucket_count() == 7);
        assert(c.size() == 6);
        C::const_iterator i = c.cbegin();
        assert(i->first == 1);
        assert(i->second == "one");
        ++i;
        assert(i->first == 1);
        assert(i->second == "four");
        ++i;
        assert(i->first == 2);
        assert(i->second == "two");
        ++i;
        assert(i->first == 2);
        assert(i->second == "four");
        ++i;
        assert(i->first == 3);
        assert(i->second == "three");
        ++i;
        assert(i->first == 4);
        assert(i->second == "four");
        assert(c.hash_function() == test_hash<std::hash<int> >(8));
        assert(c.key_eq() == test_compare<std::equal_to<int> >(9));
        assert(c.get_allocator() == A());
        assert(!c.empty());
        assert(momo::internal::UIntMath<>::Dist(c.begin(), c.end()) == c.size());
        assert(momo::internal::UIntMath<>::Dist(c.cbegin(), c.cend()) == c.size());
        assert(fabs(c.load_factor() - static_cast<float>(c.size())/c.bucket_count()) < FLT_EPSILON);
        assert(c.max_load_factor() == 1);
    }
#endif
}

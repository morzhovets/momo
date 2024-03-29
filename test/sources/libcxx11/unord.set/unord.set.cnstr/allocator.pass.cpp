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

// explicit unordered_set(const allocator_type& __a);

//#include <unordered_set>
//#include <cassert>

//#include "../../../NotConstructible.h"
//#include "../../../test_compare.h"
//#include "../../../test_hash.h"
//#include "test_allocator.h"
//#include "min_allocator.h"

void main()
{
    {
        typedef unordered_set<NotConstructible,
                                   test_hash<std::hash<NotConstructible> >,
                                   test_compare<std::equal_to<NotConstructible> >,
                                   test_allocator<NotConstructible>
                                   > C;
        C c(test_allocator<NotConstructible>(10));
        assert(c.bucket_count() == 0);
        assert(c.hash_function() == test_hash<std::hash<NotConstructible> >());
        assert(c.key_eq() == test_compare<std::equal_to<NotConstructible> >());
        assert(c.get_allocator() == test_allocator<NotConstructible>(10));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c.max_load_factor() == 1);
#endif
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef unordered_set<NotConstructible,
                                   test_hash<std::hash<NotConstructible> >,
                                   test_compare<std::equal_to<NotConstructible> >,
                                   min_allocator<NotConstructible>
                                   > C;
        C c(min_allocator<NotConstructible>{});
        assert(c.bucket_count() == 0);
        assert(c.hash_function() == test_hash<std::hash<NotConstructible> >());
        assert(c.key_eq() == test_compare<std::equal_to<NotConstructible> >());
        assert(c.get_allocator() == min_allocator<NotConstructible>());
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
        assert(c.max_load_factor() == 1);
    }
#endif
//#if _LIBCPP_STD_VER > 11
    {
        typedef NotConstructible T;
        typedef test_hash<std::hash<T>> HF;
        typedef test_compare<std::equal_to<T>> Comp;
        typedef test_allocator<T> A;
        typedef unordered_set<T, HF, Comp, A> C;

        A a(43);
        C c(3, a);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c.bucket_count() == 3);
#else
        assert(c.bucket_count() == 0);
#endif
        assert(c.hash_function() == HF());
        assert(c.key_eq() == Comp ());
        assert(c.get_allocator() == a);
        assert(!(c.get_allocator() == A()));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c.max_load_factor() == 1);
#endif
    }
    {
        typedef NotConstructible T;
        typedef test_hash<std::hash<T>> HF;
        typedef test_compare<std::equal_to<T>> Comp;
        typedef test_allocator<T> A;
        typedef unordered_set<T, HF, Comp, A> C;

        HF hf(42);
        A a(43);
        C c(4, hf, a);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c.bucket_count() == 4);
#else
        assert(c.bucket_count() == 0);
#endif
        assert(c.hash_function() == hf);
        assert(!(c.hash_function() == HF()));
        assert(c.key_eq() == Comp ());
        assert(c.get_allocator() == a);
        assert(!(c.get_allocator() == A()));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c.max_load_factor() == 1);
#endif
    }
//#endif
}

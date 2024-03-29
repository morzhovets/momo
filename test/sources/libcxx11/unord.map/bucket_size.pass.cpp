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
// class unordered_map

// size_type bucket_size(size_type n) const

//#include <unordered_map>
//#include <string>
//#include <cassert>

//#include "min_allocator.h"

void main()
{
    {
        typedef unordered_map<int, std::string, LibcppIntHash> C;
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
        const C c(std::begin(a), std::end(a));
        assert(c.bucket_count() >= 5);
        assert(c.bucket_size(0) == 0);
        assert(c.bucket_size(1) == 1);
        assert(c.bucket_size(2) == 1);
        assert(c.bucket_size(3) == 1);
        assert(c.bucket_size(4) == 1);
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
        const C c(std::begin(a), std::end(a));
        assert(c.bucket_count() >= 5);
        assert(c.bucket_size(0) == 0);
        assert(c.bucket_size(1) == 1);
        assert(c.bucket_size(2) == 1);
        assert(c.bucket_size(3) == 1);
        assert(c.bucket_size(4) == 1);
    }
#endif
#ifdef LIBCXX_TEST_FAILURE
    {
        typedef unordered_map<int, std::string> C;
        C c;
        LIBCPP_CATCH(c.bucket_size(3));
        //assert(false);
    }
#endif
}

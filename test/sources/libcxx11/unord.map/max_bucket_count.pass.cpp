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

// size_type max_bucket_count() const;

//#include <unordered_map>
//#include <string>
//#include <cassert>

//#include "min_allocator.h"

void main()
{
    {
        typedef unordered_map<int, std::string> C;
        typedef C::const_iterator I;
        typedef std::pair<int, std::string> P;
        const C c;
        assert(c.max_bucket_count() > 0);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
                            min_allocator<std::pair<const int, std::string>>> C;
        typedef C::const_iterator I;
        typedef std::pair<int, std::string> P;
        const C c;
        assert(c.max_bucket_count() > 0);
    }
#endif
}

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

// size_type max_size() const;

//#include <unordered_map>
//#include <cassert>

//#include "min_allocator.h"

void main()
{
    {
        unordered_map<int, int> u;
        assert(u.max_size() > 0);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        unordered_map<int, int, std::hash<int>, std::equal_to<int>,
                                    min_allocator<std::pair<const int, int>>> u;
        assert(u.max_size() > 0);
    }
#endif
}

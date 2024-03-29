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

// size_type max_size() const;

//#include <unordered_set>
//#include <cassert>

//#include "min_allocator.h"

void main()
{
    {
        unordered_set<int> u;
        assert(u.max_size() > 0);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        unordered_set<int, std::hash<int>,
                                      std::equal_to<int>, min_allocator<int>> u;
        assert(u.max_size() > 0);
    }
#endif
}

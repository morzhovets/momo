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

// iterator erase(const_iterator first, const_iterator last)

//#include <unordered_set>
//#include <cassert>

//#include "min_allocator.h"

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
        C::const_iterator i = c.find(2);
        C::const_iterator j = next(i);
        C::iterator k = c.erase(i, i);
        assert(k == i);
        assert(c.size() == 4);
        assert(c.count(1) == 1);
        assert(c.count(2) == 1);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);

        k = c.erase(i, j);
        assert(c.size() == 3);
        assert(c.count(1) == 1);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);

        k = c.erase(c.cbegin(), c.cend());
        assert(c.size() == 0);
        assert(k == c.end());
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
        C c(a, a + sizeof(a)/sizeof(a[0]));
        C::const_iterator i = c.find(2);
        C::const_iterator j = next(i);
        C::iterator k = c.erase(i, i);
        assert(k == i);
        assert(c.size() == 4);
        assert(c.count(1) == 1);
        assert(c.count(2) == 1);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);

        k = c.erase(i, j);
        assert(c.size() == 3);
        assert(c.count(1) == 1);
        assert(c.count(3) == 1);
        assert(c.count(4) == 1);

        k = c.erase(c.cbegin(), c.cend());
        assert(c.size() == 0);
        assert(k == c.end());
    }
#endif
}

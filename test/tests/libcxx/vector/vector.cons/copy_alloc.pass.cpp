//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <vector>

// vector(const vector& v, const allocator_type& a);

//#include <vector>
//#include <cassert>

//#include "test_macros.h"
//#include "test_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

template <class C>
TEST_CONSTEXPR_CXX20 void
test(const C& x, const typename C::allocator_type& a)
{
    typename C::size_type s = x.size();
    C c(x, a);
    //LIBCPP_ASSERT(c.__invariants());
    assert(c.size() == s);
    assert(c == x);
    //LIBCPP_ASSERT(is_contiguous_container_asan_correct(c));
}

TEST_CONSTEXPR_CXX20 bool tests() {
    {
        int a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 7, 6, 5, 4, 3, 1, 0};
        int* an = a + sizeof(a)/sizeof(a[0]);
        test(vector<int>(a, an), std::allocator<int>());
    }
    {
        vector<int, test_allocator<int> > l(3, 2, test_allocator<int>(5));
        vector<int, test_allocator<int> > l2(l, test_allocator<int>(3));
        assert(l2 == l);
        assert(l2.get_allocator() == test_allocator<int>(3));
    }
    {
        vector<int, other_allocator<int> > l(3, 2, other_allocator<int>(5));
        vector<int, other_allocator<int> > l2(l, other_allocator<int>(3));
        assert(l2 == l);
        assert(l2.get_allocator() == other_allocator<int>(3));
    }
    {
        // Test copy ctor with allocator and empty source
        vector<int, other_allocator<int> > l(other_allocator<int>(5));
        vector<int, other_allocator<int> > l2(l, other_allocator<int>(3));
        assert(l2 == l);
        assert(l2.get_allocator() == other_allocator<int>(3));
        assert(l2.empty());
    }
#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        int a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 7, 6, 5, 4, 3, 1, 0};
        int* an = a + sizeof(a)/sizeof(a[0]);
        test(vector<int, min_allocator<int>>(a, an), min_allocator<int>());
    }
    {
        vector<int, min_allocator<int> > l(3, 2, min_allocator<int>());
        vector<int, min_allocator<int> > l2(l, min_allocator<int>());
        assert(l2 == l);
        assert(l2.get_allocator() == min_allocator<int>());
    }
#endif
#endif

    return true;
}

void main()
{
    tests();
//#if TEST_STD_VER > 17
//    static_assert(tests());
//#endif
}

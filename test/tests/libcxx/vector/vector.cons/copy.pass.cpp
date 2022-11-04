//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// vector(const vector& v);

//#include <vector>
//#include <cassert>
//#include "test_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

template <class C>
void
test(const C& x)
{
    size_t s = x.size();
    C c(x);
    //assert(c.__invariants());
    assert(c.size() == s);
    assert(c == x);
    assert(!(c != x));
    assert(c <= x);
    assert(c >= x);
    assert(!(c < x));
    assert(!(c > x));
    //assert(is_contiguous_container_asan_correct(c));
}

void main()
{
    {
        int a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 7, 6, 5, 4, 3, 1, 0};
        int* an = a + sizeof(a)/sizeof(a[0]);
        test(vector<int>(a, an));
    }
    {
        vector<int, test_allocator<int> > v(3, 2, test_allocator<int>(5));
        vector<int, test_allocator<int> > v2 = v;
        //assert(is_contiguous_container_asan_correct(v));
        //assert(is_contiguous_container_asan_correct(v2));
        assert(v2 == v);
        assert(v2.get_allocator() == v.get_allocator());
        //assert(is_contiguous_container_asan_correct(v));
        //assert(is_contiguous_container_asan_correct(v2));
    }
#ifndef _LIBCPP_HAS_NO_ADVANCED_SFINAE
    {
        vector<int, other_allocator<int> > v(3, 2, other_allocator<int>(5));
        vector<int, other_allocator<int> > v2 = v;
        //assert(is_contiguous_container_asan_correct(v));
        //assert(is_contiguous_container_asan_correct(v2));
        assert(v2 == v);
        assert(v2.get_allocator() == other_allocator<int>(-2));
        //assert(is_contiguous_container_asan_correct(v));
        //assert(is_contiguous_container_asan_correct(v2));
    }
#endif  // _LIBCPP_HAS_NO_ADVANCED_SFINAE
#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        int a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 7, 6, 5, 4, 3, 1, 0};
        int* an = a + sizeof(a)/sizeof(a[0]);
        test(vector<int, min_allocator<int>>(a, an));
    }
    {
        vector<int, min_allocator<int> > v((size_t)3, 2, min_allocator<int>());
        vector<int, min_allocator<int> > v2 = v;
        //assert(is_contiguous_container_asan_correct(v));
        //assert(is_contiguous_container_asan_correct(v2));
        assert(v2 == v);
        assert(v2.get_allocator() == v.get_allocator());
        //assert(is_contiguous_container_asan_correct(v));
        //assert(is_contiguous_container_asan_correct(v2));
    }
#endif
#endif
}

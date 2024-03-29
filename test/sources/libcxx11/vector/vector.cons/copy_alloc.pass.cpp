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

// <vector>

// vector(const vector& v, const allocator_type& a);

//#include <vector>
//#include <cassert>
//#include "test_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

template <class C>
void
test(const C& x, const typename C::allocator_type& a)
{
    size_t s = x.size();
    C c(x, a);
    //assert(c.__invariants());
    assert(c.size() == s);
    assert(c == x);
    //assert(is_contiguous_container_asan_correct(c));
}

void main()
{
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
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        int a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 7, 6, 5, 4, 3, 1, 0};
        int* an = a + sizeof(a)/sizeof(a[0]);
        test(vector<int, min_allocator<int>>(a, an), min_allocator<int>());
    }
    {
        vector<int, min_allocator<int> > l((size_t)3, 2, min_allocator<int>());
        vector<int, min_allocator<int> > l2(l, min_allocator<int>());
        assert(l2 == l);
        assert(l2.get_allocator() == min_allocator<int>());
    }
#endif
}

//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// vector& operator=(const vector& c);

//#include <vector>
//#include <cassert>
//#include "test_allocator.h"
//#include "min_allocator.h"

void main()
{
    {
        vector<int, test_allocator<int> > l((size_t)3, 2, test_allocator<int>(5));
        vector<int, test_allocator<int> > l2(l, test_allocator<int>(3));
        l2 = l;
        assert(l2 == l);
        assert(l2.get_allocator() == test_allocator<int>(3));
    }
    {
        vector<int, other_allocator<int> > l((size_t)3, 2, other_allocator<int>(5));
        vector<int, other_allocator<int> > l2(l, other_allocator<int>(3));
        l2 = l;
        assert(l2 == l);
        assert(l2.get_allocator() == other_allocator<int>(5));
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int> > l((size_t)3, 2, min_allocator<int>());
        vector<int, min_allocator<int> > l2(l, min_allocator<int>());
        l2 = l;
        assert(l2 == l);
        assert(l2.get_allocator() == min_allocator<int>());
    }
#endif
}

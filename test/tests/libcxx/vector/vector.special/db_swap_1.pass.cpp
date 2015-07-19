//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// template <class T, class Alloc>
//   void swap(vector<T,Alloc>& x, vector<T,Alloc>& y);

#if _LIBCPP_DEBUG >= 1
#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))
#endif

//#include <vector>
//#include <cassert>

//#include "min_allocator.h"

void main()
{
#if _LIBCPP_DEBUG >= 1
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        vector<int> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
        vector<int> c2(a2, a2+sizeof(a2)/sizeof(a2[0]));
        vector<int>::iterator i1 = c1.begin();
        vector<int>::iterator i2 = c2.begin();
        swap(c1, c2);
        c1.erase(i2);
        c2.erase(i1);
        LIBCPP_CATCH(c1.erase(i1));
        //assert(false);
    }
#endif
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        vector<int, min_allocator<int>> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
        vector<int, min_allocator<int>> c2(a2, a2+sizeof(a2)/sizeof(a2[0]));
        vector<int, min_allocator<int>>::iterator i1 = c1.begin();
        vector<int, min_allocator<int>>::iterator i2 = c2.begin();
        swap(c1, c2);
        c1.erase(i2);
        c2.erase(i1);
        c1.erase(i1);
        assert(false);
    }
#endif
#endif
}

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

// template <class T, class Alloc>
//   void swap(vector<T,Alloc>& x, vector<T,Alloc>& y);

//#include <vector>
//#include <iterator>
//#include <cassert>
//#include "test_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

void main()
{
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        vector<int> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
        vector<int> c2(a2, a2+sizeof(a2)/sizeof(a2[0]));
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
        swap(c1, c2);
        assert(c1 == vector<int>(a2, a2+sizeof(a2)/sizeof(a2[0])));
        assert(c2 == vector<int>(a1, a1+sizeof(a1)/sizeof(a1[0])));
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        vector<int> c1(a1, a1);
        vector<int> c2(a2, a2+sizeof(a2)/sizeof(a2[0]));
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
        swap(c1, c2);
        assert(c1 == vector<int>(a2, a2+sizeof(a2)/sizeof(a2[0])));
        assert(c2.empty());
        assert(std::distance(c2.begin(), c2.end()) == 0);
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        vector<int> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
        vector<int> c2(a2, a2);
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
        swap(c1, c2);
        assert(c1.empty());
        assert(std::distance(c1.begin(), c1.end()) == 0);
        assert(c2 == vector<int>(a1, a1+sizeof(a1)/sizeof(a1[0])));
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        vector<int> c1(a1, a1);
        vector<int> c2(a2, a2);
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
        swap(c1, c2);
        assert(c1.empty());
        assert(std::distance(c1.begin(), c1.end()) == 0);
        assert(c2.empty());
        assert(std::distance(c2.begin(), c2.end()) == 0);
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        typedef test_allocator<int> A;
        vector<int, A> c1(a1, a1+sizeof(a1)/sizeof(a1[0]), A(1));
        vector<int, A> c2(a2, a2+sizeof(a2)/sizeof(a2[0]), A(2));
        swap(c1, c2);
        assert((c1 == vector<int, A>(a2, a2+sizeof(a2)/sizeof(a2[0]))));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c1.get_allocator() == A(1));
#else
        assert(c1.get_allocator() == A(2));
#endif
        assert((c2 == vector<int, A>(a1, a1+sizeof(a1)/sizeof(a1[0]))));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c2.get_allocator() == A(2));
#else
        assert(c2.get_allocator() == A(1));
#endif
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        typedef other_allocator<int> A;
        vector<int, A> c1(a1, a1+sizeof(a1)/sizeof(a1[0]), A(1));
        vector<int, A> c2(a2, a2+sizeof(a2)/sizeof(a2[0]), A(2));
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
        swap(c1, c2);
        assert((c1 == vector<int, A>(a2, a2+sizeof(a2)/sizeof(a2[0]))));
        assert(c1.get_allocator() == A(2));
        assert((c2 == vector<int, A>(a1, a1+sizeof(a1)/sizeof(a1[0]))));
        assert(c2.get_allocator() == A(1));
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        vector<int, min_allocator<int>> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
        vector<int, min_allocator<int>> c2(a2, a2+sizeof(a2)/sizeof(a2[0]));
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
        swap(c1, c2);
        assert((c1 == vector<int, min_allocator<int>>(a2, a2+sizeof(a2)/sizeof(a2[0]))));
        assert((c2 == vector<int, min_allocator<int>>(a1, a1+sizeof(a1)/sizeof(a1[0]))));
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        vector<int, min_allocator<int>> c1(a1, a1);
        vector<int, min_allocator<int>> c2(a2, a2+sizeof(a2)/sizeof(a2[0]));
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
        swap(c1, c2);
        assert((c1 == vector<int, min_allocator<int>>(a2, a2+sizeof(a2)/sizeof(a2[0]))));
        assert(c2.empty());
        assert(distance(c2.begin(), c2.end()) == 0);
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        vector<int, min_allocator<int>> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
        vector<int, min_allocator<int>> c2(a2, a2);
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
        swap(c1, c2);
        assert(c1.empty());
        assert(distance(c1.begin(), c1.end()) == 0);
        assert((c2 == vector<int, min_allocator<int>>(a1, a1+sizeof(a1)/sizeof(a1[0]))));
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        vector<int, min_allocator<int>> c1(a1, a1);
        vector<int, min_allocator<int>> c2(a2, a2);
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
        swap(c1, c2);
        assert(c1.empty());
        assert(distance(c1.begin(), c1.end()) == 0);
        assert(c2.empty());
        assert(distance(c2.begin(), c2.end()) == 0);
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        typedef min_allocator<int> A;
        vector<int, A> c1(a1, a1+sizeof(a1)/sizeof(a1[0]), A());
        vector<int, A> c2(a2, a2+sizeof(a2)/sizeof(a2[0]), A());
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
        swap(c1, c2);
        assert((c1 == vector<int, A>(a2, a2+sizeof(a2)/sizeof(a2[0]))));
        assert(c1.get_allocator() == A());
        assert((c2 == vector<int, A>(a1, a1+sizeof(a1)/sizeof(a1[0]))));
        assert(c2.get_allocator() == A());
        //assert(is_contiguous_container_asan_correct(c1));
        //assert(is_contiguous_container_asan_correct(c2));
    }
#endif
}

//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// vector(vector&& c);

//#include <vector>
//#include <cassert>
//#include "MoveOnly.h"
//#include "test_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

void main()
{
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    {
        vector<MoveOnly, test_allocator<MoveOnly> > l(test_allocator<MoveOnly>(5));
        vector<MoveOnly, test_allocator<MoveOnly> > lo(test_allocator<MoveOnly>(5));
        //assert(is_contiguous_container_asan_correct(l));
        //assert(is_contiguous_container_asan_correct(lo));
        for (int i = 1; i <= 3; ++i)
        {
            l.push_back(i);
            lo.push_back(i);
        }
        //assert(is_contiguous_container_asan_correct(l));
        //assert(is_contiguous_container_asan_correct(lo));
        vector<MoveOnly, test_allocator<MoveOnly> > l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        //assert(is_contiguous_container_asan_correct(l2));
    }
    {
        vector<MoveOnly, other_allocator<MoveOnly> > l(other_allocator<MoveOnly>(5));
        vector<MoveOnly, other_allocator<MoveOnly> > lo(other_allocator<MoveOnly>(5));
        //assert(is_contiguous_container_asan_correct(l));
        //assert(is_contiguous_container_asan_correct(lo));
        for (int i = 1; i <= 3; ++i)
        {
            l.push_back(i);
            lo.push_back(i);
        }
        //assert(is_contiguous_container_asan_correct(l));
        //assert(is_contiguous_container_asan_correct(lo));
        vector<MoveOnly, other_allocator<MoveOnly> > l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        //assert(is_contiguous_container_asan_correct(l2));
    }
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    {
        int a1[] = {1, 3, 7, 9, 10};
        vector<int> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
        //assert(is_contiguous_container_asan_correct(c1));
        vector<int>::const_iterator i = c1.begin();
        vector<int> c2 = std::move(c1);
        //assert(is_contiguous_container_asan_correct(c2));
        vector<int>::iterator j = c2.erase(i);
        assert(*j == 3);
        //assert(is_contiguous_container_asan_correct(c2));
    }
#endif
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<MoveOnly, min_allocator<MoveOnly> > l(min_allocator<MoveOnly>{});
        vector<MoveOnly, min_allocator<MoveOnly> > lo(min_allocator<MoveOnly>{});
        //assert(is_contiguous_container_asan_correct(l));
        //assert(is_contiguous_container_asan_correct(lo));
        for (int i = 1; i <= 3; ++i)
        {
            l.push_back(i);
            lo.push_back(i);
        }
        //assert(is_contiguous_container_asan_correct(l));
        //assert(is_contiguous_container_asan_correct(lo));
        vector<MoveOnly, min_allocator<MoveOnly> > l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        //assert(is_contiguous_container_asan_correct(l2));
    }
    {
        int a1[] = {1, 3, 7, 9, 10};
        vector<int, min_allocator<int>> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
        //assert(is_contiguous_container_asan_correct(c1));
        vector<int, min_allocator<int>>::const_iterator i = c1.begin();
        vector<int, min_allocator<int>> c2 = std::move(c1);
        //assert(is_contiguous_container_asan_correct(c2));
        vector<int, min_allocator<int>>::iterator j = c2.erase(i);
        assert(*j == 3);
        //assert(is_contiguous_container_asan_correct(c2));
    }
#endif
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
}

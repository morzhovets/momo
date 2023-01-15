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

// iterator erase(const_iterator first, const_iterator last);

//#include <vector>
//#include <iterator>
//#include <cassert>

//#include "min_allocator.h"
//#include "asan_testing.h"

void main()
{
    int a1[] = {1, 2, 3};
    {
        vector<int> l1(a1, a1+3);
        //assert(is_contiguous_container_asan_correct(l1));
        vector<int>::iterator i = l1.erase(l1.cbegin(), l1.cbegin());
        assert(l1.size() == 3);
        assert(std::distance(l1.cbegin(), l1.cend()) == 3);
        assert(i == l1.begin());
        //assert(is_contiguous_container_asan_correct(l1));
    }
    {
        vector<int> l1(a1, a1+3);
        //assert(is_contiguous_container_asan_correct(l1));
        vector<int>::iterator i = l1.erase(l1.cbegin(), std::next(l1.cbegin()));
        assert(l1.size() == 2);
        assert(std::distance(l1.cbegin(), l1.cend()) == 2);
        assert(i == l1.begin());
        assert(l1 == vector<int>(a1+1, a1+3));
        //assert(is_contiguous_container_asan_correct(l1));
    }
    {
        vector<int> l1(a1, a1+3);
        //assert(is_contiguous_container_asan_correct(l1));
        vector<int>::iterator i = l1.erase(l1.cbegin(), std::next(l1.cbegin(), 2));
        assert(l1.size() == 1);
        assert(std::distance(l1.cbegin(), l1.cend()) == 1);
        assert(i == l1.begin());
        assert(l1 == vector<int>(a1+2, a1+3));
        //assert(is_contiguous_container_asan_correct(l1));
    }
    {
        vector<int> l1(a1, a1+3);
        //assert(is_contiguous_container_asan_correct(l1));
        vector<int>::iterator i = l1.erase(l1.cbegin(), std::next(l1.cbegin(), 3));
        assert(l1.size() == 0);
        assert(std::distance(l1.cbegin(), l1.cend()) == 0);
        assert(i == l1.begin());
        //assert(is_contiguous_container_asan_correct(l1));
    }
    {
        vector<vector<int> > outer(2, vector<int>(1));
        //assert(is_contiguous_container_asan_correct(outer));
        //assert(is_contiguous_container_asan_correct(outer[0]));
        //assert(is_contiguous_container_asan_correct(outer[1]));
        outer.erase(outer.begin(), outer.begin());
        assert(outer.size() == 2);
        assert(outer[0].size() == 1);
        assert(outer[1].size() == 1);
        //assert(is_contiguous_container_asan_correct(outer));
        //assert(is_contiguous_container_asan_correct(outer[0]));
        //assert(is_contiguous_container_asan_correct(outer[1]));
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> l1(a1, a1+3);
        //assert(is_contiguous_container_asan_correct(l1));
        vector<int, min_allocator<int>>::iterator i = l1.erase(l1.cbegin(), l1.cbegin());
        assert(l1.size() == 3);
        assert(distance(l1.cbegin(), l1.cend()) == 3);
        assert(i == l1.begin());
        //assert(is_contiguous_container_asan_correct(l1));
    }
    {
        vector<int, min_allocator<int>> l1(a1, a1+3);
        //assert(is_contiguous_container_asan_correct(l1));
        vector<int, min_allocator<int>>::iterator i = l1.erase(l1.cbegin(), next(l1.cbegin()));
        assert(l1.size() == 2);
        assert(distance(l1.cbegin(), l1.cend()) == 2);
        assert(i == l1.begin());
        assert((l1 == vector<int, min_allocator<int>>(a1+1, a1+3)));
        //assert(is_contiguous_container_asan_correct(l1));
    }
    {
        vector<int, min_allocator<int>> l1(a1, a1+3);
        //assert(is_contiguous_container_asan_correct(l1));
        vector<int, min_allocator<int>>::iterator i = l1.erase(l1.cbegin(), next(l1.cbegin(), 2));
        assert(l1.size() == 1);
        assert(distance(l1.cbegin(), l1.cend()) == 1);
        assert(i == l1.begin());
        assert((l1 == vector<int, min_allocator<int>>(a1+2, a1+3)));
        //assert(is_contiguous_container_asan_correct(l1));
    }
    {
        vector<int, min_allocator<int>> l1(a1, a1+3);
        //assert(is_contiguous_container_asan_correct(l1));
        vector<int, min_allocator<int>>::iterator i = l1.erase(l1.cbegin(), next(l1.cbegin(), 3));
        assert(l1.size() == 0);
        assert(distance(l1.cbegin(), l1.cend()) == 0);
        assert(i == l1.begin());
        //assert(is_contiguous_container_asan_correct(l1));
    }
    {
        vector<vector<int, min_allocator<int>>, min_allocator<vector<int, min_allocator<int>>>> outer(2, vector<int, min_allocator<int>>(1));
        //assert(is_contiguous_container_asan_correct(outer));
        //assert(is_contiguous_container_asan_correct(outer[0]));
        //assert(is_contiguous_container_asan_correct(outer[1]));
        outer.erase(outer.begin(), outer.begin());
        assert(outer.size() == 2);
        assert(outer[0].size() == 1);
        assert(outer[1].size() == 1);
        //assert(is_contiguous_container_asan_correct(outer));
        //assert(is_contiguous_container_asan_correct(outer[0]));
        //assert(is_contiguous_container_asan_correct(outer[1]));
    }
#endif
}

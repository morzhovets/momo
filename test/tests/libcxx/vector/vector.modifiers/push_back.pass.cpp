//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// void push_back(const value_type& x);

//#include <vector>
//#include <cassert>
//#include "../../../stack_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

void main()
{
    {
        vector<int> c;
        c.push_back(0);
        assert(c.size() == 1);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
        c.push_back(1);
        assert(c.size() == 2);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
        c.push_back(2);
        assert(c.size() == 3);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
        c.push_back(3);
        assert(c.size() == 4);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
        c.push_back(4);
        assert(c.size() == 5);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
    }
#ifdef LIBCPP_TEST_STACK_ALLOCATOR
    {
        vector<int, stack_allocator<int, 15> > c;
        c.push_back(0);
        assert(c.size() == 1);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == (int)j);
        c.push_back(1);
        assert(c.size() == 2);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == (int)j);
        c.push_back(2);
        assert(c.size() == 3);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == (int)j);
        c.push_back(3);
        assert(c.size() == 4);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == (int)j);
        c.push_back(4);
        assert(c.size() == 5);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == (int)j);
    }
#endif
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> c;
        c.push_back(0);
        assert(c.size() == 1);
        //assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; j < c.size(); ++j)
            assert(c[j] == (int)j);
        c.push_back(1);
        assert(c.size() == 2);
        //assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; j < c.size(); ++j)
            assert(c[j] == (int)j);
        c.push_back(2);
        assert(c.size() == 3);
        //assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; j < c.size(); ++j)
            assert(c[j] == (int)j);
        c.push_back(3);
        assert(c.size() == 4);
        //assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; j < c.size(); ++j)
            assert(c[j] == (int)j);
        c.push_back(4);
        assert(c.size() == 5);
        //assert(is_contiguous_container_asan_correct(c));
        for (int j = 0; j < c.size(); ++j)
            assert(c[j] == (int)j);
    }
#endif
}

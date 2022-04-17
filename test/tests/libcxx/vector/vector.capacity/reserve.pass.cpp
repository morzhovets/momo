//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// void reserve(size_type n);

//#include <vector>
//#include <cassert>
//#include "../../../stack_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

void main()
{
    {
        vector<int> v;
        v.reserve(10);
        assert(v.capacity() >= 10);
        //assert(is_contiguous_container_asan_correct(v));
    }
    {
        vector<int> v(100);
        assert(v.capacity() == 100);
        v.reserve(50);
        assert(v.size() == 100);
        assert(v.capacity() == 100);
        v.reserve(150);
        assert(v.size() == 100);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(v.capacity() == 150);
#else
        assert(v.capacity() >= 150);
#endif
        //assert(is_contiguous_container_asan_correct(v));
    }
#ifdef LIBCPP_TEST_STACK_ALLOCATOR
    {
        vector<int, stack_allocator<int, 250> > v(100);
        assert(v.capacity() == 100);
        v.reserve(50);
        assert(v.size() == 100);
        assert(v.capacity() == 100);
        v.reserve(150);
        assert(v.size() == 100);
        assert(v.capacity() == 150);
        //assert(is_contiguous_container_asan_correct(v));
    }
#endif
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> v;
        v.reserve(10);
        assert(v.capacity() >= 10);
        //assert(is_contiguous_container_asan_correct(v));
    }
    {
        vector<int, min_allocator<int>> v(100);
        assert(v.capacity() == 100);
        v.reserve(50);
        assert(v.size() == 100);
        assert(v.capacity() == 100);
        v.reserve(150);
        assert(v.size() == 100);
        assert(v.capacity() == 150);
        //assert(is_contiguous_container_asan_correct(v));
    }
#endif
}

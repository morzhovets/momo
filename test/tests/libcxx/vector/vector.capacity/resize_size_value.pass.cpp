//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// void resize(size_type sz, const value_type& x);

//#include <vector>
//#include <cassert>
//#include "../../../stack_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

void main()
{
    {
        vector<int> v(100);
        v.resize(50, 1);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        assert(v == vector<int>(50));
        v.resize(200, 1);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        //assert(is_contiguous_container_asan_correct(v));
        for (unsigned i = 0; i < 50; ++i)
            assert(v[i] == 0);
        for (unsigned i = 50; i < 200; ++i)
            assert(v[i] == 1);
    }
#ifdef LIBCPP_TEST_STACK_ALLOCATOR
    {
        vector<int, stack_allocator<int, 300> > v(100);
        v.resize(50, 1);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        v.resize(200, 1);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        //assert(is_contiguous_container_asan_correct(v));
    }
#endif
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> v(100);
        v.resize(50, 1);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        //assert(is_contiguous_container_asan_correct(v));
        assert((v == vector<int, min_allocator<int>>(50)));
        v.resize(200, 1);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        //assert(is_contiguous_container_asan_correct(v));
        for (unsigned i = 0; i < 50; ++i)
            assert(v[i] == 0);
        for (unsigned i = 50; i < 200; ++i)
            assert(v[i] == 1);
    }
    {
        vector<int, min_allocator<int>> v(100);
        v.resize(50, 1);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        //assert(is_contiguous_container_asan_correct(v));
        v.resize(200, 1);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        //assert(is_contiguous_container_asan_correct(v));
    }
#endif
}

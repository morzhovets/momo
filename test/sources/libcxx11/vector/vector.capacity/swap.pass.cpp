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

// void swap(vector& x);

//#include <vector>
//#include <cassert>

//#include "min_allocator.h"
//#include "asan_testing.h"

void main()
{
    {
        vector<int> v1(100);
        vector<int> v2(200);
        //assert(is_contiguous_container_asan_correct(v1));
        //assert(is_contiguous_container_asan_correct(v2));
        v1.swap(v2);
        assert(v1.size() == 200);
        assert(v1.capacity() == 200);
        //assert(is_contiguous_container_asan_correct(v1));
        assert(v2.size() == 100);
        assert(v2.capacity() == 100);
        //assert(is_contiguous_container_asan_correct(v2));
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> v1(100);
        vector<int, min_allocator<int>> v2(200);
        //assert(is_contiguous_container_asan_correct(v1));
        //assert(is_contiguous_container_asan_correct(v2));
        v1.swap(v2);
        assert(v1.size() == 200);
        assert(v1.capacity() == 200);
        //assert(is_contiguous_container_asan_correct(v1));
        assert(v2.size() == 100);
        assert(v2.capacity() == 100);
        //assert(is_contiguous_container_asan_correct(v2));
    }
#endif
}

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

// void shrink_to_fit();

//#include <vector>
//#include <cassert>
//#include "../../../stack_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

void main()
{
    {
        vector<int> v(100);
        v.push_back(1);
        //assert(is_contiguous_container_asan_correct(v));
        v.shrink_to_fit();
        assert(v.capacity() == 101);
        assert(v.size() == 101);
        //assert(is_contiguous_container_asan_correct(v));
    }
#ifdef LIBCPP_TEST_STACK_ALLOCATOR
    {
        vector<int, stack_allocator<int, 401> > v(100);
        v.push_back(1);
        //assert(is_contiguous_container_asan_correct(v));
        v.shrink_to_fit();
        assert(v.capacity() == 101);
        assert(v.size() == 101);
        //assert(is_contiguous_container_asan_correct(v));
    }
#ifndef _LIBCPP_NO_EXCEPTIONS
    {
        vector<int, stack_allocator<int, 400> > v(100);
        v.push_back(1);
        //assert(is_contiguous_container_asan_correct(v));
        LIBCPP_CATCH(v.shrink_to_fit());
        assert(v.capacity() == 200);
        assert(v.size() == 101);
        //assert(is_contiguous_container_asan_correct(v));
    }
#endif
#endif
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> v(100);
        v.push_back(1);
        //assert(is_contiguous_container_asan_correct(v));
        v.shrink_to_fit();
        assert(v.capacity() == 101);
        assert(v.size() == 101);
        //assert(is_contiguous_container_asan_correct(v));
    }
#endif
}

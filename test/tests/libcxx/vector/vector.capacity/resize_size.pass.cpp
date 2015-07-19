//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// void resize(size_type sz);

//#include <vector>
//#include <cassert>
//#include "../../../stack_allocator.h"
//#include "MoveOnly.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

void main()
{
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    {
        vector<MoveOnly> v(100);
        v.resize(50);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        //assert(is_contiguous_container_asan_correct(v));
        v.resize(200);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        //assert(is_contiguous_container_asan_correct(v));
    }
#ifdef LIBCPP_TEST_STACK_ALLOCATOR
    {
        vector<MoveOnly, stack_allocator<MoveOnly, 300> > v(100);
        v.resize(50);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        //assert(is_contiguous_container_asan_correct(v));
        v.resize(200);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        //assert(is_contiguous_container_asan_correct(v));
    }
#endif
#else  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
    {
        vector<int> v(100);
        v.resize(50);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        //assert(is_contiguous_container_asan_correct(v));
        v.resize(200);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        //assert(is_contiguous_container_asan_correct(v));
    }
#ifdef LIBCPP_TEST_STACK_ALLOCATOR
    {
        vector<int, stack_allocator<int, 300> > v(100);
        v.resize(50);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        //assert(is_contiguous_container_asan_correct(v));
        v.resize(200);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        //assert(is_contiguous_container_asan_correct(v));
    }
#endif
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<MoveOnly, min_allocator<MoveOnly>> v(100);
        v.resize(50);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        //assert(is_contiguous_container_asan_correct(v));
        v.resize(200);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        //assert(is_contiguous_container_asan_correct(v));
    }
#endif
}

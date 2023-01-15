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

// template <class... Args> iterator emplace(const_iterator pos, Args&&... args);

//#include <vector>
//#include <cassert>

//#include "min_allocator.h"
//#include "asan_testing.h"

void main()
{
#ifndef _LIBCPP_HAS_NO_GENERALIZED_INITIALIZERS
    {
        vector<int> v;
        v.reserve(3);
        //assert(is_contiguous_container_asan_correct(v));
        v = { 1, 2, 3 };
        v.emplace(v.begin(), v.back());
        assert(v[0] == 3);
        //assert(is_contiguous_container_asan_correct(v));
    }
    {
        vector<int> v;
        v.reserve(4);
        //assert(is_contiguous_container_asan_correct(v));
        v = { 1, 2, 3 };
        v.emplace(v.begin(), v.back());
        assert(v[0] == 3);
        //assert(is_contiguous_container_asan_correct(v));
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> v;
        v.reserve(3);
        //assert(is_contiguous_container_asan_correct(v));
        v = { 1, 2, 3 };
        v.emplace(v.begin(), v.back());
        assert(v[0] == 3);
        //assert(is_contiguous_container_asan_correct(v));
    }
    {
        vector<int, min_allocator<int>> v;
        v.reserve(4);
        //assert(is_contiguous_container_asan_correct(v));
        v = { 1, 2, 3 };
        v.emplace(v.begin(), v.back());
        assert(v[0] == 3);
        //assert(is_contiguous_container_asan_correct(v));
    }
#endif
#endif  // _LIBCPP_HAS_NO_GENERALIZED_INITIALIZERS
}

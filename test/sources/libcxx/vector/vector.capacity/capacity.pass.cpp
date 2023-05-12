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

// size_type capacity() const;

//#include <vector>
//#include <cassert>

//#include "min_allocator.h"
//#include "asan_testing.h"

void main()
{
#ifndef LIBCXX_TEST_INTCAP_ARRAY
    {
        vector<int> v;
        assert(v.capacity() == 0);
        //assert(is_contiguous_container_asan_correct(v));
    }
#endif
    {
        vector<int> v(100);
        assert(v.capacity() == 100);
        v.push_back(0);
        assert(v.capacity() > 101);
        //assert(is_contiguous_container_asan_correct(v));
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> v;
        assert(v.capacity() == 0);
        //assert(is_contiguous_container_asan_correct(v));
    }
    {
        vector<int, min_allocator<int>> v(100);
        assert(v.capacity() == 100);
        v.push_back(0);
        assert(v.capacity() > 101);
        //assert(is_contiguous_container_asan_correct(v));
    }
#endif
}

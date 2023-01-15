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

// pointer data();

//#include <vector>
//#include <cassert>

//#include "min_allocator.h"
//#include "asan_testing.h"

void main()
{
#ifndef LIBCXX_TEST_INTCAP_ARRAY
    {
        vector<int> v;
        assert(v.data() == nullptr);
        //assert(is_contiguous_container_asan_correct(v));
    }
#endif
    {
        vector<int> v(100);
        assert(v.data() == &v.front());
        //assert(is_contiguous_container_asan_correct(v));
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> v;
        assert(v.data() == 0);
        //assert(is_contiguous_container_asan_correct(v));
    }
    {
        vector<int, min_allocator<int>> v(100);
        assert(v.data() == &v.front());
        //assert(is_contiguous_container_asan_correct(v));
    }
#endif
}

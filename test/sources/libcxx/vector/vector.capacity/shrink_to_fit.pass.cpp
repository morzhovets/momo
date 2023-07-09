//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
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
//#include "test_macros.h"
//#include "test_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

TEST_CONSTEXPR_CXX20 bool tests() {
#ifdef LIBCXX_TEST_SEGMENTED_ARRAY
    {
        vector<int> v(101);
        v.reserve(200);
        assert(v.capacity() >= 200);
        v.shrink_to_fit();
        assert(v.capacity() < 200);
        assert(v.size() == 101);
    }
#else
    {
        vector<int> v(100);
        v.push_back(1);
        //assert(is_contiguous_container_asan_correct(v));
        v.shrink_to_fit();
        assert(v.capacity() == 101);
        assert(v.size() == 101);
        //assert(is_contiguous_container_asan_correct(v));
    }
    {
        vector<int, limited_allocator<int, 401> > v(100);
        v.push_back(1);
        //assert(is_contiguous_container_asan_correct(v));
        v.shrink_to_fit();
        assert(v.capacity() == 101);
        assert(v.size() == 101);
        //assert(is_contiguous_container_asan_correct(v));
    }
#ifndef TEST_HAS_NO_EXCEPTIONS
    if (!TEST_IS_CONSTANT_EVALUATED) {
        vector<int, limited_allocator<int, 400> > v(100);
        v.push_back(1);
        //assert(is_contiguous_container_asan_correct(v));
        v.shrink_to_fit();
        LIBCPP_ASSERT(v.capacity() == 200); // assumes libc++'s 2x growth factor
        assert(v.size() == 101);
        //assert(is_contiguous_container_asan_correct(v));
    }
#endif
#endif
#if TEST_STD_VER >= 11
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
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
    {
      vector<int, safe_allocator<int>> v(100);
      v.push_back(1);
      //assert(is_contiguous_container_asan_correct(v));
      v.shrink_to_fit();
      assert(v.capacity() == 101);
      assert(v.size() == 101);
      //assert(is_contiguous_container_asan_correct(v));
    }
#endif
#endif

    return true;
}

void main()
{
    tests();
//#if TEST_STD_VER > 17
//    static_assert(tests());
//#endif
}

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

// void swap(vector& x);

//#include <vector>
//#include <cassert>

//#include "test_macros.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

TEST_CONSTEXPR_CXX20 bool tests() {
    {
        vector<int> v1(100);
        vector<int> v2(200);
        assert(is_contiguous_container_asan_correct(v1));
        assert(is_contiguous_container_asan_correct(v2));
        v1.swap(v2);
        assert(v1.size() == 200);
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
        assert(v1.capacity() == 200);
#endif
        assert(is_contiguous_container_asan_correct(v1));
        assert(v2.size() == 100);
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
        assert(v2.capacity() == 100);
#endif
        assert(is_contiguous_container_asan_correct(v2));
    }
#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> v1(100);
        vector<int, min_allocator<int>> v2(200);
        assert(is_contiguous_container_asan_correct(v1));
        assert(is_contiguous_container_asan_correct(v2));
        v1.swap(v2);
        assert(v1.size() == 200);
        assert(v1.capacity() == 200);
        assert(is_contiguous_container_asan_correct(v1));
        assert(v2.size() == 100);
        assert(v2.capacity() == 100);
        assert(is_contiguous_container_asan_correct(v2));
    }
#endif
    {
      vector<int, safe_allocator<int>> v1(100);
      vector<int, safe_allocator<int>> v2(200);
      assert(is_contiguous_container_asan_correct(v1));
      assert(is_contiguous_container_asan_correct(v2));
      v1.swap(v2);
      assert(v1.size() == 200);
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
      assert(v1.capacity() == 200);
#endif
      assert(is_contiguous_container_asan_correct(v1));
      assert(v2.size() == 100);
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
      assert(v2.capacity() == 100);
#endif
      assert(is_contiguous_container_asan_correct(v2));
    }
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

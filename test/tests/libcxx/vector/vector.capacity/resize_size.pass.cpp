//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <vector>

// void resize(size_type sz);

//#include <vector>
//#include <cassert>

//#include "test_macros.h"
//#include "test_allocator.h"
//#include "MoveOnly.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

TEST_CONSTEXPR_CXX20 bool tests() {
    {
        vector<int> v(100);
        v.resize(50);
        assert(v.size() == 50);
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
        assert(v.capacity() == 100);
#endif
        //assert(is_contiguous_container_asan_correct(v));
        v.resize(200);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        //assert(is_contiguous_container_asan_correct(v));
    }
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
    {
        // Add 1 for implementations that dynamically allocate a container proxy.
        vector<int, limited_allocator<int, 300 * sizeof(int) + 1> > v(100);
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
#if TEST_STD_VER >= 11
    {
        vector<MoveOnly> v(100);
        v.resize(50);
        assert(v.size() == 50);
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
        assert(v.capacity() == 100);
#endif
        //assert(is_contiguous_container_asan_correct(v));
        v.resize(200);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        //assert(is_contiguous_container_asan_correct(v));
    }
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
    {
        // Add 1 for implementations that dynamically allocate a container proxy.
        vector<MoveOnly, limited_allocator<MoveOnly, 300 * sizeof(MoveOnly) + 1> > v(100);
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

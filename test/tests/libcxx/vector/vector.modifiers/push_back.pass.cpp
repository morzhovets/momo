//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <vector>

// void push_back(const value_type& x);

//#include <vector>
//#include <cassert>
//#include <cstddef>
//#include "test_macros.h"
//#include "test_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

TEST_CONSTEXPR_CXX20 bool tests()
{
    {
        vector<int> c;
        c.push_back(0);
        assert(c.size() == 1);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
        c.push_back(1);
        assert(c.size() == 2);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
        c.push_back(2);
        assert(c.size() == 3);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
        c.push_back(3);
        assert(c.size() == 4);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
        c.push_back(4);
        assert(c.size() == 5);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
    }
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
    {
        // libc++ needs 15 because it grows by 2x (1 + 2 + 4 + 8).
        // Use 17 for implementations that dynamically allocate a container proxy
        // and grow by 1.5x (1 for proxy + 1 + 2 + 3 + 4 + 6).
        vector<int, limited_allocator<int, 17 * sizeof(int)> > c;
        c.push_back(0);
        assert(c.size() == 1);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
        c.push_back(1);
        assert(c.size() == 2);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
        c.push_back(2);
        assert(c.size() == 3);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
        c.push_back(3);
        assert(c.size() == 4);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
        c.push_back(4);
        assert(c.size() == 5);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
    }
#endif
//#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> c;
        c.push_back(0);
        assert(c.size() == 1);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
        c.push_back(1);
        assert(c.size() == 2);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
        c.push_back(2);
        assert(c.size() == 3);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
        c.push_back(3);
        assert(c.size() == 4);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
        c.push_back(4);
        assert(c.size() == 5);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == static_cast<int>(j));
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

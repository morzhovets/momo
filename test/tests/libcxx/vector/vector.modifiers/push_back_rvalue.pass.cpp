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

// UNSUPPORTED: c++03 && !stdlib=libc++

// <vector>

// void push_back(value_type&& x);

//#include <vector>
//#include <cassert>
//#include <cstddef>
//#include "test_macros.h"
//#include "MoveOnly.h"
//#include "test_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

TEST_CONSTEXPR_CXX20 bool tests()
{
    {
        vector<MoveOnly> c;
        c.push_back(MoveOnly(0));
        assert(c.size() == 1);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
        c.push_back(MoveOnly(1));
        assert(c.size() == 2);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
        c.push_back(MoveOnly(2));
        assert(c.size() == 3);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
        c.push_back(MoveOnly(3));
        assert(c.size() == 4);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
        c.push_back(MoveOnly(4));
        assert(c.size() == 5);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
    }
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
    {
        // libc++ needs 15 because it grows by 2x (1 + 2 + 4 + 8).
        // Use 17 for implementations that dynamically allocate a container proxy
        // and grow by 1.5x (1 for proxy + 1 + 2 + 3 + 4 + 6).
        vector<MoveOnly, limited_allocator<MoveOnly, 17> > c;
        c.push_back(MoveOnly(0));
        assert(c.size() == 1);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
        c.push_back(MoveOnly(1));
        assert(c.size() == 2);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
        c.push_back(MoveOnly(2));
        assert(c.size() == 3);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
        c.push_back(MoveOnly(3));
        assert(c.size() == 4);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
        c.push_back(MoveOnly(4));
        assert(c.size() == 5);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
    }
#endif
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<MoveOnly, min_allocator<MoveOnly>> c;
        c.push_back(MoveOnly(0));
        assert(c.size() == 1);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
        c.push_back(MoveOnly(1));
        assert(c.size() == 2);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
        c.push_back(MoveOnly(2));
        assert(c.size() == 3);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
        c.push_back(MoveOnly(3));
        assert(c.size() == 4);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
        c.push_back(MoveOnly(4));
        assert(c.size() == 5);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
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

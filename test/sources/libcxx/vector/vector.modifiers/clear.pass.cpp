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

// void clear() noexcept;

//#include <vector>
//#include <cassert>

//#include "test_macros.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

TEST_CONSTEXPR_CXX20 bool tests()
{
    {
    int a[] = {1, 2, 3};
    vector<int> c(a, a+3);
    //ASSERT_NOEXCEPT(c.clear());
    c.clear();
    assert(c.empty());
    //LIBCPP_ASSERT(c.__invariants());
    LIBCPP_ASSERT(is_contiguous_container_asan_correct(c));
    }
#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    int a[] = {1, 2, 3};
    vector<int, min_allocator<int>> c(a, a+3);
    ASSERT_NOEXCEPT(c.clear());
    c.clear();
    assert(c.empty());
    //LIBCPP_ASSERT(c.__invariants());
    LIBCPP_ASSERT(is_contiguous_container_asan_correct(c));
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

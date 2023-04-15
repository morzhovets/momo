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

// void pop_back();

//#include <vector>
//#include <cassert>

//#include "test_macros.h"
//#include "test_allocator.h"
//#include "min_allocator.h"


TEST_CONSTEXPR_CXX20 bool tests()
{
    {
        vector<int> c;
        c.push_back(1);
        assert(c.size() == 1);
        c.pop_back();
        assert(c.size() == 0);
#if _LIBCPP_DEBUG >= 1
        LIBCPP_CATCH(c.pop_back());
        //assert(false);
#endif
    }
#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> c;
        c.push_back(1);
        assert(c.size() == 1);
        c.pop_back();
        assert(c.size() == 0);
#if _LIBCPP_DEBUG >= 1
        c.pop_back();
        assert(false);
#endif
#endif
#endif

    { // LWG 526
        int arr[] = {0, 1, 2, 3, 4};
        int sz = 5;
        vector<int> c(arr, arr+sz);
        while (c.size() < c.capacity())
            c.push_back(sz++);
        c.push_back(c.front());
        assert(c.back() == 0);
        for (int i = 0; i < sz; ++i)
            assert(c[static_cast<size_t>(i)] == i);
    }

    return true;
}

void main()
{
    tests();
//#if TEST_STD_VER > 17
//    static_assert(tests());
//#endif
}

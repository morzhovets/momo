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

// void clear() noexcept;

//#include <vector>
//#include <cassert>

//#include "test_macros.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

void main()
{
    {
    int a[] = {1, 2, 3};
    vector<int> c(a, a+3);
    //ASSERT_NOEXCEPT(c.clear());
    c.clear();
    assert(c.empty());
    //LIBCPP_ASSERT(c.__invariants());
    //LIBCPP_ASSERT(is_contiguous_container_asan_correct(c));
    }
//#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    int a[] = {1, 2, 3};
    vector<int, min_allocator<int>> c(a, a+3);
    ASSERT_NOEXCEPT(c.clear());
    c.clear();
    assert(c.empty());
    LIBCPP_ASSERT(c.__invariants());
    LIBCPP_ASSERT(is_contiguous_container_asan_correct(c));
    }
#endif
}

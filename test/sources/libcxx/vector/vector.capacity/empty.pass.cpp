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

// class vector

// bool empty() const noexcept;

//#include <vector>
//#include <cassert>

//#include "test_macros.h"
//#include "min_allocator.h"

TEST_CONSTEXPR_CXX20 bool tests() {
    {
    typedef vector<int> C;
    C c;
    ASSERT_NOEXCEPT(c.empty());
    assert(c.empty());
    c.push_back(C::value_type(1));
    assert(!c.empty());
    c.clear();
    assert(c.empty());
    }
#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef vector<int, min_allocator<int>> C;
    C c;
    ASSERT_NOEXCEPT(c.empty());
    assert(c.empty());
    c.push_back(C::value_type(1));
    assert(!c.empty());
    c.clear();
    assert(c.empty());
    }
#endif
    {
      typedef vector<int, safe_allocator<int>> C;
      C c;
      ASSERT_NOEXCEPT(c.empty());
      assert(c.empty());
      c.push_back(C::value_type(1));
      assert(!c.empty());
      c.clear();
      assert(c.empty());
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

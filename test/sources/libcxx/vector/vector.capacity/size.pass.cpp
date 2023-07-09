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

// size_type size() const noexcept;

//#include <vector>
//#include <cassert>

//#include "test_macros.h"
//#include "min_allocator.h"

TEST_CONSTEXPR_CXX20 bool tests()
{
    {
    typedef vector<int> C;
    C c;
    ASSERT_NOEXCEPT(c.size());
    assert(c.size() == 0);
    c.push_back(C::value_type(2));
    assert(c.size() == 1);
    c.push_back(C::value_type(1));
    assert(c.size() == 2);
    c.push_back(C::value_type(3));
    assert(c.size() == 3);
    c.erase(c.begin());
    assert(c.size() == 2);
    c.erase(c.begin());
    assert(c.size() == 1);
    c.erase(c.begin());
    assert(c.size() == 0);
    }
#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef vector<int, min_allocator<int>> C;
    C c;
    ASSERT_NOEXCEPT(c.size());
    assert(c.size() == 0);
    c.push_back(C::value_type(2));
    assert(c.size() == 1);
    c.push_back(C::value_type(1));
    assert(c.size() == 2);
    c.push_back(C::value_type(3));
    assert(c.size() == 3);
    c.erase(c.begin());
    assert(c.size() == 2);
    c.erase(c.begin());
    assert(c.size() == 1);
    c.erase(c.begin());
    assert(c.size() == 0);
    }
#endif
    {
      typedef vector<int, safe_allocator<int>> C;
      C c;
      ASSERT_NOEXCEPT(c.size());
      assert(c.size() == 0);
      c.push_back(C::value_type(2));
      assert(c.size() == 1);
      c.push_back(C::value_type(1));
      assert(c.size() == 2);
      c.push_back(C::value_type(3));
      assert(c.size() == 3);
      c.erase(c.begin());
      assert(c.size() == 2);
      c.erase(c.begin());
      assert(c.size() == 1);
      c.erase(c.begin());
      assert(c.size() == 0);
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

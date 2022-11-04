//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <vector>

// size_type max_size() const;

//#include <cassert>
//#include <limits>
//#include <type_traits>
//#include <vector>

//#include "test_allocator.h"
//#include "test_macros.h"


TEST_CONSTEXPR_CXX20 bool test() {
  {
    typedef limited_allocator<int, 10> A;
    typedef vector<int, A> C;
    C c;
    assert(c.max_size() <= 10);
    LIBCPP_ASSERT(c.max_size() == 10);
  }
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
  {
    typedef limited_allocator<int, (size_t)-1> A;
    typedef vector<int, A> C;
    const C::size_type max_dist =
        static_cast<C::size_type>(std::numeric_limits<C::difference_type>::max());
    C c;
    assert(c.max_size() <= max_dist);
    LIBCPP_ASSERT(c.max_size() == max_dist);
  }
  {
    typedef vector<char> C;
    const C::size_type max_dist =
        static_cast<C::size_type>(std::numeric_limits<C::difference_type>::max());
    C c;
    assert(c.max_size() <= max_dist);
    assert(c.max_size() <= alloc_max_size(c.get_allocator()));
  }
#endif

  return true;
}

void main() {
  test();
//#if TEST_STD_VER > 17
//  static_assert(test());
//#endif
}

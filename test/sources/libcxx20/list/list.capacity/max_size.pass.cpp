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

// <list>

// size_type max_size() const noexcept // constexpr since C++26

TEST_CONSTEXPR_CXX26 bool test() {
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
  {
    typedef limited_allocator<int, 10> A;
    typedef std::list<int, A> C;
    C c;
    assert(c.max_size() <= 10);
    LIBCPP_ASSERT(c.max_size() == 10);
  }
#endif
  {
    typedef limited_allocator<int, static_cast<std::size_t>(-1)> A;
    typedef std::list<int, A> C;
    const C::size_type max_dist = static_cast<C::size_type>(std::numeric_limits<C::difference_type>::max());
    C c;
    assert(c.max_size() <= max_dist);
    LIBCPP_ASSERT(c.max_size() == max_dist);
  }
  {
    typedef std::list<char> C;
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    const C::size_type max_dist = static_cast<C::size_type>(std::numeric_limits<C::difference_type>::max());
#endif
    C c;
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(c.max_size() <= max_dist);
#endif
    assert(c.max_size() <= alloc_max_size(c.get_allocator()));
  }

  return true;
}

int main(int, char**) {
  assert(test());
#if TEST_STD_VER >= 26
  static_assert(test());
#endif

  return 0;
}

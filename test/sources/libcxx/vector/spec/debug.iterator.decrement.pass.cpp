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

// Decrement iterator prior to begin.

// REQUIRES: has-unix-headers
// UNSUPPORTED: !libcpp-has-legacy-debug-mode, c++03

int main(int, char**) {
  {
    typedef int T;
    typedef std::vector<T> C;
    C c(1);
    C::iterator i = c.end();
    --i;
    assert(i == c.begin());
    TEST_LIBCPP_ASSERT_FAILURE(--i, "Attempted to decrement a non-decrementable iterator");
  }

  {
    typedef int T;
    typedef std::vector<T, min_allocator<T> > C;
    C c(1);
    C::iterator i = c.end();
    --i;
    assert(i == c.begin());
    TEST_LIBCPP_ASSERT_FAILURE(--i, "Attempted to decrement a non-decrementable iterator");
  }

  return 0;
}

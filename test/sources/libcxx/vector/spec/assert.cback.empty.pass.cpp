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

// Call back() on empty const container.

// REQUIRES: has-unix-headers
// UNSUPPORTED: c++03
// XFAIL: availability-verbose_abort-missing
// ADDITIONAL_COMPILE_FLAGS: -D_LIBCPP_ENABLE_ASSERTIONS=1

int main(int, char**) {
  {
    typedef int T;
    typedef std::vector<T, min_allocator<T> > C;
    const C c;
    TEST_LIBCPP_ASSERT_FAILURE(c.back(), "back() called on an empty vector");
  }

  {
    typedef int T;
    typedef std::vector<T> C;
    const C c;
    TEST_LIBCPP_ASSERT_FAILURE(c.back(), "back() called on an empty vector");
  }

  return 0;
}

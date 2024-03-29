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

// Index iterator out of bounds.

// REQUIRES: has-unix-headers
// UNSUPPORTED: !libcpp-has-legacy-debug-mode, c++03

int main(int, char**) {
  {
    typedef int T;
    typedef std::vector<T> C;
    C c(1);
    C::iterator i = c.begin();
    assert(i[0] == 0);
    TEST_LIBCPP_ASSERT_FAILURE(i[1], "Attempted to subscript an iterator outside its valid range");
  }

  {
    typedef int T;
    typedef std::vector<T, min_allocator<T> > C;
    C c(1);
    C::iterator i = c.begin();
    assert(i[0] == 0);
    TEST_LIBCPP_ASSERT_FAILURE(i[1], "Attempted to subscript an iterator outside its valid range");
  }

  return 0;
}

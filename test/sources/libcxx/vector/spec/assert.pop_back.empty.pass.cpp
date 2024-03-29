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

// pop_back() more than the number of elements in a vector

// REQUIRES: has-unix-headers
// UNSUPPORTED: c++03
// XFAIL: availability-verbose_abort-missing
// ADDITIONAL_COMPILE_FLAGS: -D_LIBCPP_ENABLE_ASSERTIONS=1

int main(int, char**) {
    std::vector<int> v;
    v.push_back(0);
    v.pop_back();
    TEST_LIBCPP_ASSERT_FAILURE(v.pop_back(), "vector::pop_back called on an empty vector");

    return 0;
}

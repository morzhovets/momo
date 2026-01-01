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

// <unordered_map>

// class unordered_map

// mapped_type& at(const key_type& k);

// Make sure we abort() when exceptions are disabled and we fetch a key that
// is not in the map.

// REQUIRES: no-exceptions
// UNSUPPORTED: c++03


int main(int, char**) {
    std::unordered_map<int, int> map;
    TEST_LIBCPP_ASSERT_FAILURE(map.at(1), "");
    return 0;
}

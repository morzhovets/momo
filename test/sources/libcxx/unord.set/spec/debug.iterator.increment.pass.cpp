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

// <unordered_set>

// Increment iterator past end.

// REQUIRES: has-unix-headers
// UNSUPPORTED: !libcpp-has-legacy-debug-mode, c++03

int main(int, char**) {
    {
        typedef int T;
        typedef std::unordered_set<T> C;
        C c;
        c.insert(42);
        C::iterator i = c.begin();
        assert(i != c.end());
        ++i;
        assert(i == c.end());
        TEST_LIBCPP_ASSERT_FAILURE(++i, "Attempted to increment a non-incrementable unordered container const_iterator");
    }

    {
        typedef int T;
        typedef std::unordered_set<T, std::hash<T>, std::equal_to<T>, min_allocator<T>> C;
        C c({42});
        C::iterator i = c.begin();
        assert(i != c.end());
        ++i;
        assert(i == c.end());
        TEST_LIBCPP_ASSERT_FAILURE(++i, "Attempted to increment a non-incrementable unordered container const_iterator");
    }

    return 0;
}

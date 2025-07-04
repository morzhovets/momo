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

// iterator insert(const_iterator p, const value_type& x);

// REQUIRES: has-unix-headers
// UNSUPPORTED: !libcpp-has-legacy-debug-mode, c++03

int main(int, char**) {
#ifdef LIBCXX_TEST_HINT_ITERATORS
    typedef std::unordered_set<double> C;
    typedef C::value_type P;
    C c;
    C c2;
    C::const_iterator e = c2.end();
    P v(3.5);
    TEST_LIBCPP_ASSERT_FAILURE(
        c.insert(e, v),
        "unordered_set::insert(const_iterator, const value_type&) called with an iterator not referring to this unordered_set");
#endif

    return 0;
}

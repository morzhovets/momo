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

// template <class P,
//           class = typename enable_if<is_convertible<P, value_type>::value>::type>
//     iterator insert(const_iterator p, P&& x);

// REQUIRES: has-unix-headers
// UNSUPPORTED: !libcpp-has-legacy-debug-mode, c++03

int main(int, char**) {
#ifdef LIBCXX_TEST_HINT_ITERATORS
    typedef std::unordered_map<double, int> C;
    typedef C::value_type P;
    C c;
    C c2;
    C::const_iterator e = c2.end();
    TEST_LIBCPP_ASSERT_FAILURE(
        c.insert(e, P(3.5, 3)),
        "unordered_map::insert(const_iterator, value_type&&) called with an iterator not referring to this unordered_map");
#endif

    return 0;
}

// -*- C++ -*-
//===---------------------------- test_macros.h ---------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef SUPPORT_TEST_MACROS_HPP
#define SUPPORT_TEST_MACROS_HPP

#define TEST_IS_CONSTANT_EVALUATED std::is_constant_evaluated()

#define TEST_NOEXCEPT noexcept

#define TEST_CONSTEXPR //constexpr

#define TEST_CONSTEXPR_CXX14 //constexpr
#define TEST_CONSTEXPR_CXX20 //constexpr

#define TEST_NODISCARD [[nodiscard]]

#define TEST_IGNORE_NODISCARD (void)

#define TEST_THROW(...) throw __VA_ARGS__

namespace test_macros_detail {
template <class T, class U>
struct is_same { enum { value = 0};} ;
template <class T>
struct is_same<T, T> { enum {value = 1}; };
} // namespace test_macros_detail

#define ASSERT_SAME_TYPE(...) \
    static_assert((test_macros_detail::is_same<__VA_ARGS__>::value), \
                 "Types differ unexpectedly")

#endif // SUPPORT_TEST_MACROS_HPP

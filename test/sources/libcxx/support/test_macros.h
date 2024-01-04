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

#define ASSERT_NOEXCEPT(...) \
    static_assert(noexcept(__VA_ARGS__), "Operation must be noexcept")

#define LIBCPP_ASSERT(...) static_assert(true, "")
#define LIBCPP_STATIC_ASSERT(...) static_assert(true, "")

#define ASSERT_SAME_TYPE(...) \
    static_assert((std::is_same_v<__VA_ARGS__>), \
                 "Types differ unexpectedly")

#define TEST_HAS_NO_INT128

#define TEST_ALIGNOF(...) alignof(__VA_ARGS__)

#define TEST_STRINGIZE_IMPL(x) #x
#define TEST_STRINGIZE(x) TEST_STRINGIZE_IMPL(x)

#if defined(TEST_MSVC)
#define TEST_DIAGNOSTIC_PUSH _Pragma("warning(push)")
#define TEST_DIAGNOSTIC_POP _Pragma("warning(pop)")
#define TEST_MSVC_DIAGNOSTIC_IGNORED(num) _Pragma(TEST_STRINGIZE(warning(disable: num)))
#else
#define TEST_DIAGNOSTIC_PUSH
#define TEST_DIAGNOSTIC_POP
#define TEST_MSVC_DIAGNOSTIC_IGNORED(num)
#endif

#endif // SUPPORT_TEST_MACROS_HPP

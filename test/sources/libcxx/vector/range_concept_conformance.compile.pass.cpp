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

// UNSUPPORTED: c++03, c++11, c++14, c++17

// vector

using range = std::vector<int>;


static_assert(std::same_as<std::ranges::iterator_t<range>, range::iterator>);
static_assert(std::ranges::common_range<range>);
static_assert(std::ranges::random_access_range<range>);
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
static_assert(std::ranges::contiguous_range<range>);
#endif
static_assert(!std::ranges::view<range>);
static_assert(std::ranges::sized_range<range>);
static_assert(!std::ranges::borrowed_range<range>);
#if !defined(TEST_GCC) && !defined(TEST_CLANG)
static_assert(std::ranges::viewable_range<range>);
#endif

static_assert(std::same_as<std::ranges::iterator_t<range const>, range::const_iterator>);
static_assert(std::ranges::common_range<range const>);
static_assert(std::ranges::random_access_range<range const>);
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
static_assert(std::ranges::contiguous_range<range const>);
#endif
static_assert(!std::ranges::view<range const>);
static_assert(std::ranges::sized_range<range const>);
static_assert(!std::ranges::borrowed_range<range const>);
#if !defined(TEST_GCC) && !defined(TEST_CLANG)
static_assert(!std::ranges::viewable_range<range const>);
#endif

void main()
{
}

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

// <set>

// class set

// template<class Key, class Compare, class Allocator>
//   synth-three-way-result<Key> operator<=>(const set<Key, Compare, Allocator>& x,
//                                           const set<Key, Compare, Allocator>& y);

int main(int, char**) {
  assert((test_ordered_set_container_spaceship<std::set>()));
  // `std::set` is not constexpr, so no `static_assert` test here.
  return 0;
}

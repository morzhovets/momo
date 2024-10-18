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

// template <class Key, class T, class Hash = hash<Key>, class Pred = equal_to<Key>,
//           class Alloc = allocator<pair<const Key, T>>>
// class unordered_multimap

// hasher key_eq() const;

int main(int, char**) {
  typedef std::unordered_multimap<int, std::string> map_type;

  map_type m;
  map_type::iterator i1 = m.insert(map_type::value_type(1, "abc"));
  map_type::iterator i2 = m.insert(map_type::value_type(1, "bcd"));
  map_type::iterator i3 = m.insert(map_type::value_type(2, "abc"));

#ifndef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
  i1 = m.find(1);
  i2 = std::next(i1);
#endif

  const map_type& cm = m;

  assert(cm.key_eq()(i1->first, i1->first));
  assert(cm.key_eq()(i2->first, i2->first));
  assert(cm.key_eq()(i3->first, i3->first));

  assert(cm.key_eq()(i1->first, i2->first));
  assert(cm.key_eq()(i2->first, i1->first));

  assert(!cm.key_eq()(i1->first, i3->first));
  assert(!cm.key_eq()(i3->first, i1->first));

  assert(!cm.key_eq()(i2->first, i3->first));
  assert(!cm.key_eq()(i3->first, i2->first));

  return 0;
}

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

// hasher hash_function() const;

int main(int, char**) {
  typedef std::unordered_multimap<int, std::string> map_type;
  map_type m;

  map_type::iterator i1 = m.insert(map_type::value_type(1, "abc"));
  map_type::iterator i2 = m.insert(map_type::value_type(1, "bcd"));

#ifndef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
  i1 = m.find(1);
#endif

  const map_type& cm = m;
  assert(cm.hash_function()(i1->first) == cm.hash_function()(i2->first));
  assert(cm.hash_function()(i2->first) == cm.hash_function()(i1->first));

  return 0;
}

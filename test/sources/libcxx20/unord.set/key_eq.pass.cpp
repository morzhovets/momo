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

// template <class Value, class Hash = hash<Value>, class Pred = equal_to<Value>,
//           class Alloc = allocator<Value>>
// class unordered_set

// key_equal key_eq() const;

int main(int, char**) {
  typedef std::unordered_set<int> set_type;

  set_type s;

  std::pair<set_type::iterator, bool> p1 = s.insert(1);
  std::pair<set_type::iterator, bool> p2 = s.insert(2);

#ifdef LIBCXX_TEST_HASH_LIST_SET
#else
  p1.first = s.find(1);
#endif

  const set_type& cs = s;

  assert(cs.key_eq()(*p1.first, *p1.first));
  assert(cs.key_eq()(*p2.first, *p2.first));

  assert(!cs.key_eq()(*p1.first, *p2.first));
  assert(!cs.key_eq()(*p2.first, *p1.first));

  return 0;
}

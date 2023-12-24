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
// class unordered_map

// https://llvm.org/PR16538
// https://llvm.org/PR16549

struct Key {
  template <typename T> Key(const T&) {}
  bool operator== (const Key&) const { return true; }
};

struct Hasher
{
    std::size_t operator()(const Key&) const { return 0; }
};

int main(int, char**)
{
    typedef std::unordered_map<Key, int, Hasher> MapT;
    typedef MapT::iterator Iter;
    MapT map;
    Iter it = map.find(Key(0));
    assert(it == map.end());
    std::pair<Iter, bool> result = map.insert(std::make_pair(Key(0), 42));
    assert(result.second);
    assert(result.first->second == 42);

  return 0;
}

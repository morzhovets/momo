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

// template <typename K>
// size_type count(const K& k) const;

// UNSUPPORTED: c++03, c++11, c++14, c++17

//#include <unordered_map>

//#include "test_transparent_unordered.h"

void main()
{
    using key_type = StoredType<int>;
    using ilist_type = std::initializer_list<std::pair<const key_type, int> >;

    {
      // Make sure conversions don't happen for transparent non-final hasher and key_equal
      using map_type = const unord_map_type<unordered_map, transparent_hash, std::equal_to<> >;
      test_transparent_count<map_type>(ilist_type{{1, 2}, {2, 3}});
    }

    {
      // Make sure conversions don't happen for transparent final hasher and key_equal
      using map_type =
          const unord_map_type<unordered_map, transparent_hash_final, transparent_equal_final>;
      test_transparent_count<map_type>(ilist_type{{1, 2}, {2, 3}});
    }

    {
      // Make sure conversions do happen for non-transparent hasher
      using map_type = const unord_map_type<unordered_map, non_transparent_hash, std::equal_to<> >;
      test_non_transparent_count<map_type>(ilist_type{{1, 2}, {2, 3}});
    }

    {
      // Make sure conversions do happen for non-transparent key_equal
      using map_type =
          const unord_map_type<unordered_map, transparent_hash, std::equal_to<key_type> >;
      test_non_transparent_count<map_type>(ilist_type{{1, 2}, {2, 3}});
    }

    {
      // Make sure conversions do happen for both non-transparent hasher and key_equal
      using map_type =
          const unord_map_type<unordered_map, non_transparent_hash, std::equal_to<key_type> >;
      test_non_transparent_count<map_type>(ilist_type{{1, 2}, {2, 3}});
    }
}

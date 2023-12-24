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

// class unordered_map

// insert(...);
// emplace(...);

// UNSUPPORTED: c++03

namespace TCT {
template <class Key = CopyInsertable<1>, class Value = CopyInsertable<2>,
          class ValueTp = std::pair<const Key, Value> >
using unordered_map =
      std::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>,
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
                              ContainerTestAllocator<ValueTp, ValueTp>
#else
                              ContainerTestAllocatorForMap<ValueTp, Key, Value>
#endif
      >;
}

int main(int, char**)
{
  testMapInsert<TCT::unordered_map<> >();
#ifndef MOMO_USE_UNORDERED_HINT_ITERATORS
  testMapInsertHint<TCT::unordered_map<> >();
#endif
  testMapEmplace<TCT::unordered_map<> >();
#ifndef MOMO_USE_UNORDERED_HINT_ITERATORS
  testMapEmplaceHint<TCT::unordered_map<> >();
#endif

  return 0;
}

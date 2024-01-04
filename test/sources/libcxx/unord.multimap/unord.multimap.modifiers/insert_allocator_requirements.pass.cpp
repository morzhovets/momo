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

// class unordered_multimap

// insert(...)

// UNSUPPORTED: c++03

namespace TCT {
template <class Key = CopyInsertable<1>, class Value = CopyInsertable<2>,
          class ValueTp = std::pair<const Key, Value> >
using unordered_multimap =
      std::unordered_multimap<Key, Value, std::hash<Key>, std::equal_to<Key>,
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
                              ContainerTestAllocator<ValueTp, ValueTp>
#else
                              ContainerTestAllocatorForMap<ValueTp, Key, Value>
#endif
      >;
}

int main(int, char**)
{
  testMultimapInsert<TCT::unordered_multimap<> >();
  testMultimapInsertHint<TCT::unordered_multimap<> >();

  return 0;
}

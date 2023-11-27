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

// <map>

// class map

// insert(...);
// emplace(...);
// emplace_hint(...);

// UNSUPPORTED: c++03

namespace TCT {
template <class Key = CopyInsertable<1>, class Value = CopyInsertable<2>,
          class ValueTp = std::pair<const Key, Value> >
using map =
      std::map<Key, Value, std::less<Key>,
                              ContainerTestAllocator<ValueTp, ValueTp> >;
}

int main(int, char**)
{
  testMapInsert<TCT::map<> >();
  testMapInsertHint<TCT::map<> >();
  testMapEmplace<TCT::map<> >();
  testMapEmplaceHint<TCT::map<> >();

  return 0;
}

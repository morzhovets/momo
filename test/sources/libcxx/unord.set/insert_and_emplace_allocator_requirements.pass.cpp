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

// class unordered_set

// insert(...)
// emplace(...)

// UNSUPPORTED: c++03

namespace TCT {
template <class Value = CopyInsertable<1> >
using unordered_set =
  std::unordered_set<Value, std::hash<Value>, std::equal_to<Value>,
                               ContainerTestAllocator<Value, Value> >;
}

int main(int, char**)
{
  testSetInsert<TCT::unordered_set<> >();
  testSetEmplace<TCT::unordered_set<> >();

  return 0;
}

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

// <set>

// class set

// insert(...)
// emplace(...)
// emplace_hint(...)

// UNSUPPORTED: c++03

namespace TCT {
template <class Value = CopyInsertable<1> >
using set =
    std::set<Value, std::less<Value>, ContainerTestAllocator<Value, Value> >;
}

int main(int, char**)
{
  testSetInsert<TCT::set<> >();
  testSetEmplace<TCT::set<> >();
  testSetEmplaceHint<TCT::set<> >();

  return 0;
}

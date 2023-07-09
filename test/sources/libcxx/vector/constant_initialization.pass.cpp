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

// UNSUPPORTED: c++03

//#include <algorithm>
//#include <vector>

vector<int> ca_allocs;

void main() {
  ca_allocs.push_back(0);
  for ([[maybe_unused]] const auto& a : ca_allocs)
    ;
}

//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Modified for https://github.com/morzhovets/momo project.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11

// <map>

// class multimap

// template<typename K>
//   size_type count(const K& x) const;        // C++14

//#include <cassert>
//#include <map>
//#include <utility>

//#include "min_allocator.h"
//#include "private_constructor.hpp"
//#include "test_macros.h"

struct Comp {
  using is_transparent = void;

  bool operator()(const std::pair<int, int> &lhs,
                  const std::pair<int, int> &rhs) const {
    return lhs < rhs;
  }

  bool operator()(const std::pair<int, int> &lhs, int rhs) const {
    return lhs.first < rhs;
  }

  bool operator()(int lhs, const std::pair<int, int> &rhs) const {
    return lhs < rhs.first;
  }
};

void main() {
  multimap<std::pair<int, int>, int, Comp> s{
      {{2, 1}, 1}, {{1, 1}, 2}, {{1, 1}, 3}, {{1, 1}, 4}, {{2, 2}, 5}};

  auto cnt = s.count(1);
  assert(cnt == 3);
}

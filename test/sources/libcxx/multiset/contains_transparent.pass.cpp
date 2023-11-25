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

// UNSUPPORTED: c++03, c++11, c++14, c++17

// <set>

// class set

// template<typename K> bool contains(const K& x) const; // C++20

struct Comp {
  using is_transparent = void;

  bool operator()(const std::pair<int, int>& lhs,
                  const std::pair<int, int>& rhs) const {
    return lhs < rhs;
  }

  bool operator()(const std::pair<int, int>& lhs, int rhs) const {
    return lhs.first < rhs;
  }

  bool operator()(int lhs, const std::pair<int, int>& rhs) const {
    return lhs < rhs.first;
  }
};

template <typename Container>
void test() {
  Container s{{2, 1}, {1, 2}, {1, 3}, {1, 4}, {2, 2}};

  assert(s.contains(1));
  assert(!s.contains(-1));
}

int main(int, char**) {
  test<std::multiset<std::pair<int, int>, Comp> >();

  return 0;
}

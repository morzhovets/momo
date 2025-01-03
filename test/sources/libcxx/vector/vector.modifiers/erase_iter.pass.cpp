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

// <vector>

// iterator erase(const_iterator position);

#ifndef TEST_HAS_NO_EXCEPTIONS
struct Throws {
  Throws() : v_(0) {}
  Throws(int v) : v_(v) {}
  Throws(const Throws& rhs) : v_(rhs.v_) {
    if (sThrows)
      throw 1;
  }
  Throws(Throws&& rhs) : v_(rhs.v_) {
    if (sThrows)
      throw 1;
  }
  Throws& operator=(const Throws& rhs) {
    v_ = rhs.v_;
    return *this;
  }
  Throws& operator=(Throws&& rhs) {
    v_ = rhs.v_;
    return *this;
  }
  int v_;
  static bool sThrows;
};

bool Throws::sThrows = false;
#endif

TEST_CONSTEXPR_CXX20 bool tests() {
  {
    int a1[] = {1, 2, 3, 4, 5};
    std::vector<int> l1(a1, a1 + 5);
    l1.erase(l1.begin());
    assert(is_contiguous_container_asan_correct(l1));
    assert(l1 == std::vector<int>(a1 + 1, a1 + 5));
  }
  {
    int a1[] = {1, 2, 3, 4, 5};
    int e1[] = {1, 3, 4, 5};
    std::vector<int> l1(a1, a1 + 5);
    l1.erase(l1.begin() + 1);
    assert(is_contiguous_container_asan_correct(l1));
    assert(l1 == std::vector<int>(e1, e1 + 4));
  }
  {
    int a1[] = {1, 2, 3};
    std::vector<int> l1(a1, a1 + 3);
    std::vector<int>::const_iterator i = l1.begin();
    assert(is_contiguous_container_asan_correct(l1));
    ++i;
    std::vector<int>::iterator j = l1.erase(i);
    assert(l1.size() == 2);
    assert(std::distance(l1.begin(), l1.end()) == 2);
    assert(*j == 3);
    assert(*l1.begin() == 1);
    assert(*std::next(l1.begin()) == 3);
    assert(is_contiguous_container_asan_correct(l1));
    j = l1.erase(j);
    assert(j == l1.end());
    assert(l1.size() == 1);
    assert(std::distance(l1.begin(), l1.end()) == 1);
    assert(*l1.begin() == 1);
    assert(is_contiguous_container_asan_correct(l1));
    j = l1.erase(l1.begin());
    assert(j == l1.end());
    assert(l1.size() == 0);
    assert(std::distance(l1.begin(), l1.end()) == 0);
    assert(is_contiguous_container_asan_correct(l1));
  }

  // Make sure vector::erase works with move-only types
  // When non-trivial
  {
    std::vector<MoveOnly> v;
    v.emplace_back(1);
    v.emplace_back(2);
    v.emplace_back(3);
    v.erase(v.begin());
    assert(v.size() == 2);
    assert(v[0] == MoveOnly(2));
    assert(v[1] == MoveOnly(3));
  }
  // When trivial
  {
    std::vector<TrivialMoveOnly> v;
    v.emplace_back(1);
    v.emplace_back(2);
    v.emplace_back(3);
    v.erase(v.begin());
    assert(v.size() == 2);
    assert(v[0] == TrivialMoveOnly(2));
    assert(v[1] == TrivialMoveOnly(3));
  }

#if TEST_STD_VER >= 11
  {
    int a1[] = {1, 2, 3};
    std::vector<int, min_allocator<int>> l1(a1, a1 + 3);
    std::vector<int, min_allocator<int>>::const_iterator i = l1.begin();
    assert(is_contiguous_container_asan_correct(l1));
    ++i;
    std::vector<int, min_allocator<int>>::iterator j = l1.erase(i);
    assert(l1.size() == 2);
    assert(std::distance(l1.begin(), l1.end()) == 2);
    assert(*j == 3);
    assert(*l1.begin() == 1);
    assert(*std::next(l1.begin()) == 3);
    assert(is_contiguous_container_asan_correct(l1));
    j = l1.erase(j);
    assert(j == l1.end());
    assert(l1.size() == 1);
    assert(std::distance(l1.begin(), l1.end()) == 1);
    assert(*l1.begin() == 1);
    assert(is_contiguous_container_asan_correct(l1));
    j = l1.erase(l1.begin());
    assert(j == l1.end());
    assert(l1.size() == 0);
    assert(std::distance(l1.begin(), l1.end()) == 0);
    assert(is_contiguous_container_asan_correct(l1));
  }
#endif

  return true;
}

int main(int, char**) {
  tests();
#if TEST_STD_VER > 17
  //static_assert(tests());
#endif

#ifndef TEST_HAS_NO_EXCEPTIONS
  // Test for LWG2853:
  // Throws: Nothing unless an exception is thrown by the assignment operator or move assignment operator of T.
  {
    Throws arr[] = {1, 2, 3};
    std::vector<Throws> v(arr, arr + 3);
    Throws::sThrows = true;
    v.erase(v.begin());
#ifdef LIBCXX_TEST_ARRAY
    v.erase(std::prev(v.end()));
#else
    v.erase(--v.end());
#endif
    v.erase(v.begin());
    assert(v.size() == 0);
  }
#endif

  return 0;
}

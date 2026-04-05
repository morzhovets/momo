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
// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: libcpp-no-deduction-guides
// UNSUPPORTED: apple-clang-9.1

// template<class InputIterator,
//          class Compare = less<iter-value-type<InputIterator>>,
//          class Allocator = allocator<iter-value-type<InputIterator>>>
// set(InputIterator, InputIterator,
//     Compare = Compare(), Allocator = Allocator())
//   -> set<iter-value-type<InputIterator>, Compare, Allocator>;
// template<class Key, class Compare = less<Key>,
//          class Allocator = allocator<Key>>
// set(initializer_list<Key>, Compare = Compare(), Allocator = Allocator())
//   -> set<Key, Compare, Allocator>;
// template<class InputIterator, class Allocator>
// set(InputIterator, InputIterator, Allocator)
//   -> set<iter-value-type<InputIterator>,
//          less<iter-value-type<InputIterator>>, Allocator>;
// template<class Key, class Allocator>
// set(initializer_list<Key>, Allocator)
//   -> set<Key, less<Key>, Allocator>;

//#include <algorithm> // std::equal
//#include <cassert>
//#include <climits> // INT_MAX
//#include <functional>
//#include <set>
//#include <type_traits>
//
//#include "test_allocator.h"

struct NotAnAllocator {
  friend bool operator<(NotAnAllocator, NotAnAllocator) { return false; }
};

void main() {
  {
    const int arr[] = { 1, 2, 1, INT_MAX, 3 };
    set s(std::begin(arr), std::end(arr));

    ASSERT_SAME_TYPE(decltype(s), set<int>);
    const int expected_s[] = { 1, 2, 3, INT_MAX };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
  }

  {
    const int arr[] = { 1, 2, 1, INT_MAX, 3 };
    set s(std::begin(arr), std::end(arr), std::greater<int>());

    ASSERT_SAME_TYPE(decltype(s), set<int, std::greater<int> >);
    const int expected_s[] = { INT_MAX, 3, 2, 1 };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
  }

  {
    const int arr[] = { 1, 2, 1, INT_MAX, 3 };
    set s(std::begin(arr), std::end(arr), std::greater<int>(),
               test_allocator<int>(0, 42));

    ASSERT_SAME_TYPE(decltype(s),
                     set<int, std::greater<int>, test_allocator<int> >);
    const int expected_s[] = { INT_MAX, 3, 2, 1 };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
    assert(s.get_allocator().get_id() == 42);
  }

  {
    set<long> source;
    set s(source);
    ASSERT_SAME_TYPE(decltype(s), set<long>);
    assert(s.size() == 0);
  }

  {
    set<long> source;
    set s{ source };  // braces instead of parens
    ASSERT_SAME_TYPE(decltype(s), set<long>);
    assert(s.size() == 0);
  }

  {
    set<long> source;
    set s(source, set<long>::allocator_type());
    ASSERT_SAME_TYPE(decltype(s), set<long>);
    assert(s.size() == 0);
  }

#if !(defined(TEST_GCC) && __GNUC__ < 13)
  {
    set s{ 1, 2, 1, INT_MAX, 3 };

    ASSERT_SAME_TYPE(decltype(s), set<int>);
    const int expected_s[] = { 1, 2, 3, INT_MAX };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
  }
#endif

  {
    set s({ 1, 2, 1, INT_MAX, 3 }, std::greater<int>());

    ASSERT_SAME_TYPE(decltype(s), set<int, std::greater<int> >);
    const int expected_s[] = { INT_MAX, 3, 2, 1 };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
  }

  {
    set s({ 1, 2, 1, INT_MAX, 3 }, std::greater<int>(),
               test_allocator<int>(0, 43));

    ASSERT_SAME_TYPE(decltype(s),
                     set<int, std::greater<int>, test_allocator<int> >);
    const int expected_s[] = { INT_MAX, 3, 2, 1 };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
    assert(s.get_allocator().get_id() == 43);
  }

  {
    const int arr[] = { 1, 2, 1, INT_MAX, 3 };
    set s(std::begin(arr), std::end(arr), test_allocator<int>(0, 44));

    ASSERT_SAME_TYPE(decltype(s),
                     set<int, std::less<int>, test_allocator<int> >);
    const int expected_s[] = { 1, 2, 3, INT_MAX };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
    assert(s.get_allocator().get_id() == 44);
  }

  {
    set s({ 1, 2, 1, INT_MAX, 3 }, test_allocator<int>(0, 45));

    ASSERT_SAME_TYPE(decltype(s),
                     set<int, std::less<int>, test_allocator<int> >);
    const int expected_s[] = { 1, 2, 3, INT_MAX };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
    assert(s.get_allocator().get_id() == 45);
  }

#if !(defined(TEST_GCC) && __GNUC__ < 13)
  {
    NotAnAllocator a;
    set s{ a }; // set(initializer_list<NotAnAllocator>)
    ASSERT_SAME_TYPE(decltype(s), set<NotAnAllocator>);
    assert(s.size() == 1);
  }

  {
    set<long> source;
    set s{ source, source }; // set(initializer_list<set<long>>)
    ASSERT_SAME_TYPE(decltype(s), set<set<long> >);
    assert(s.size() == 1);
  }

  {
    NotAnAllocator a;
    set s{ a, a }; // set(initializer_list<NotAnAllocator>)
    ASSERT_SAME_TYPE(decltype(s), set<NotAnAllocator>);
    assert(s.size() == 1);
  }
#endif

  {
    int source[3] = { 3, 4, 5 };
    set s(source, source + 3); // set(InputIterator, InputIterator)
    ASSERT_SAME_TYPE(decltype(s), set<int>);
    assert(s.size() == 3);
  }

#if !(defined(TEST_GCC) && __GNUC__ < 13)
  {
    int source[3] = { 3, 4, 5 };
    set s{ source, source + 3 }; // set(initializer_list<int*>)
    ASSERT_SAME_TYPE(decltype(s), set<int *>);
    assert(s.size() == 2);
  }
#endif
}

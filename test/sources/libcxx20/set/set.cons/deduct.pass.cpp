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

// UNSUPPORTED: c++03, c++11, c++14

// <set>

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
//
// template<ranges::input_range R, class Compare = less<ranges::range_value_t<R>>,
//          class Allocator = allocator<ranges::range_value_t<R>>>
//   set(from_range_t, R&&, Compare = Compare(), Allocator = Allocator())
//     -> set<ranges::range_value_t<R>, Compare, Allocator>; // C++23
//
// template<ranges::input_range R, class Allocator>
//   set(from_range_t, R&&, Allocator)
//     -> set<ranges::range_value_t<R>, less<ranges::range_value_t<R>>, Allocator>; // C++23

struct NotAnAllocator {
  friend bool operator<(NotAnAllocator, NotAnAllocator) { return false; }
};

int main(int, char **) {
  {
    const int arr[] = { 1, 2, 1, INT_MAX, 3 };
    LIBCXX_TEST_CLASS s(std::begin(arr), std::end(arr));

    ASSERT_SAME_TYPE(decltype(s), LIBCXX_TEST_CLASS<int>);
    const int expected_s[] = { 1, 2, 3, INT_MAX };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
  }

  {
    const int arr[] = { 1, 2, 1, INT_MAX, 3 };
    LIBCXX_TEST_CLASS s(std::begin(arr), std::end(arr), std::greater<int>());

    ASSERT_SAME_TYPE(decltype(s), LIBCXX_TEST_CLASS<int, std::greater<int> >);
    const int expected_s[] = { INT_MAX, 3, 2, 1 };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
  }

  {
    const int arr[] = { 1, 2, 1, INT_MAX, 3 };
    LIBCXX_TEST_CLASS s(std::begin(arr), std::end(arr), std::greater<int>(),
               test_allocator<int>(0, 42));

    ASSERT_SAME_TYPE(decltype(s),
                     LIBCXX_TEST_CLASS<int, std::greater<int>, test_allocator<int> >);
    const int expected_s[] = { INT_MAX, 3, 2, 1 };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
    assert(s.get_allocator().get_id() == 42);
  }

  {
    LIBCXX_TEST_CLASS<long> source;
    LIBCXX_TEST_CLASS s(source);
    ASSERT_SAME_TYPE(decltype(s), LIBCXX_TEST_CLASS<long>);
    assert(s.size() == 0);
  }

  {
    LIBCXX_TEST_CLASS<long> source;
    LIBCXX_TEST_CLASS s{ source };  // braces instead of parens
    ASSERT_SAME_TYPE(decltype(s), LIBCXX_TEST_CLASS<long>);
    assert(s.size() == 0);
  }

  {
    LIBCXX_TEST_CLASS<long> source;
    LIBCXX_TEST_CLASS s(source, LIBCXX_TEST_CLASS<long>::allocator_type());
    ASSERT_SAME_TYPE(decltype(s), LIBCXX_TEST_CLASS<long>);
    assert(s.size() == 0);
  }

  {
    LIBCXX_TEST_CLASS s{ 1, 2, 1, INT_MAX, 3 };

    ASSERT_SAME_TYPE(decltype(s), LIBCXX_TEST_CLASS<int>);
    const int expected_s[] = { 1, 2, 3, INT_MAX };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
  }

  {
    LIBCXX_TEST_CLASS s({ 1, 2, 1, INT_MAX, 3 }, std::greater<int>());

    ASSERT_SAME_TYPE(decltype(s), LIBCXX_TEST_CLASS<int, std::greater<int> >);
    const int expected_s[] = { INT_MAX, 3, 2, 1 };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
  }

  {
    LIBCXX_TEST_CLASS s({ 1, 2, 1, INT_MAX, 3 }, std::greater<int>(),
               test_allocator<int>(0, 43));

    ASSERT_SAME_TYPE(decltype(s),
                     LIBCXX_TEST_CLASS<int, std::greater<int>, test_allocator<int> >);
    const int expected_s[] = { INT_MAX, 3, 2, 1 };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
    assert(s.get_allocator().get_id() == 43);
  }

  {
    const int arr[] = { 1, 2, 1, INT_MAX, 3 };
    LIBCXX_TEST_CLASS s(std::begin(arr), std::end(arr), test_allocator<int>(0, 44));

    ASSERT_SAME_TYPE(decltype(s),
                     LIBCXX_TEST_CLASS<int, std::less<int>, test_allocator<int> >);
    const int expected_s[] = { 1, 2, 3, INT_MAX };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
    assert(s.get_allocator().get_id() == 44);
  }

  {
    LIBCXX_TEST_CLASS s({ 1, 2, 1, INT_MAX, 3 }, test_allocator<int>(0, 45));

    ASSERT_SAME_TYPE(decltype(s),
                     LIBCXX_TEST_CLASS<int, std::less<int>, test_allocator<int> >);
    const int expected_s[] = { 1, 2, 3, INT_MAX };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
    assert(s.get_allocator().get_id() == 45);
  }

  {
    NotAnAllocator a;
    LIBCXX_TEST_CLASS s{ a }; // set(initializer_list<NotAnAllocator>)
    ASSERT_SAME_TYPE(decltype(s), LIBCXX_TEST_CLASS<NotAnAllocator>);
    assert(s.size() == 1);
  }

  {
    LIBCXX_TEST_CLASS<long> source;
    LIBCXX_TEST_CLASS s{ source, source }; // set(initializer_list<set<long>>)
    ASSERT_SAME_TYPE(decltype(s), LIBCXX_TEST_CLASS<LIBCXX_TEST_CLASS<long> >);
    assert(s.size() == 1);
  }

  {
    NotAnAllocator a;
    LIBCXX_TEST_CLASS s{ a, a }; // set(initializer_list<NotAnAllocator>)
    ASSERT_SAME_TYPE(decltype(s), LIBCXX_TEST_CLASS<NotAnAllocator>);
    assert(s.size() == 1);
  }

  {
    int source[3] = { 3, 4, 5 };
    LIBCXX_TEST_CLASS s(source, source + 3); // set(InputIterator, InputIterator)
    ASSERT_SAME_TYPE(decltype(s), LIBCXX_TEST_CLASS<int>);
    assert(s.size() == 3);
  }

  {
    int source[3] = { 3, 4, 5 };
    LIBCXX_TEST_CLASS s{ source, source + 3 }; // set(initializer_list<int*>)
    ASSERT_SAME_TYPE(decltype(s), LIBCXX_TEST_CLASS<int *>);
    assert(s.size() == 2);
  }

#if TEST_STD_VER >= 23
    {
      using Range = std::array<int, 0>;
      using Comp = std::greater<int>;
      using DefaultComp = std::less<int>;
      using Alloc = test_allocator<int>;

      { // (from_range, range)
        LIBCXX_TEST_CLASS c(std::from_range, Range());
        static_assert(std::is_same_v<decltype(c), LIBCXX_TEST_CLASS<int>>);
      }

      { // (from_range, range, comp)
        LIBCXX_TEST_CLASS c(std::from_range, Range(), Comp());
        static_assert(std::is_same_v<decltype(c), LIBCXX_TEST_CLASS<int, Comp>>);
      }

      { // (from_range, range, comp, alloc)
        LIBCXX_TEST_CLASS c(std::from_range, Range(), Comp(), Alloc());
        static_assert(std::is_same_v<decltype(c), LIBCXX_TEST_CLASS<int, Comp, Alloc>>);
      }

      { // (from_range, range, alloc)
        LIBCXX_TEST_CLASS c(std::from_range, Range(), Alloc());
        static_assert(std::is_same_v<decltype(c), LIBCXX_TEST_CLASS<int, DefaultComp, Alloc>>);
      }
    }
#endif

  //AssociativeContainerDeductionGuidesSfinaeAway<std::set, std::set<int>>();
  AssociativeContainerDeductionGuidesSfinaeAway<LIBCXX_TEST_CLASS, LIBCXX_TEST_CLASS<int>>();

  return 0;
}

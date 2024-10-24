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
// multiset(InputIterator, InputIterator,
//          Compare = Compare(), Allocator = Allocator())
//   -> multiset<iter-value-type<InputIterator>, Compare, Allocator>;
// template<class Key, class Compare = less<Key>,
//          class Allocator = allocator<Key>>
// multiset(initializer_list<Key>, Compare = Compare(), Allocator = Allocator())
//   -> multiset<Key, Compare, Allocator>;
// template<class InputIterator, class Allocator>
// multiset(InputIterator, InputIterator, Allocator)
//   -> multiset<iter-value-type<InputIterator>,
//               less<iter-value-type<InputIterator>>, Allocator>;
// template<class Key, class Allocator>
// multiset(initializer_list<Key>, Allocator)
//   -> multiset<Key, less<Key>, Allocator>;
//
// template<ranges::input_range R, class Compare = less<ranges::range_value_t<R>>,
//          class Allocator = allocator<ranges::range_value_t<R>>>
//   multiset(from_range_t, R&&, Compare = Compare(), Allocator = Allocator())
//     -> multiset<ranges::range_value_t<R>, Compare, Allocator>;
//
// template<ranges::input_range R, class Allocator>
//   multiset(from_range_t, R&&, Allocator)
//     -> multiset<ranges::range_value_t<R>, less<ranges::range_value_t<R>>, Allocator>;

struct NotAnAllocator {
  friend bool operator<(NotAnAllocator, NotAnAllocator) { return false; }
};

int main(int, char **) {
  {
    const int arr[] = { 1, 2, 1, INT_MAX, 3 };
    momo::stdish::multiset s(std::begin(arr), std::end(arr));

    ASSERT_SAME_TYPE(decltype(s), momo::stdish::multiset<int>);
    const int expected_s[] = { 1, 1, 2, 3, INT_MAX };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
  }

  {
    const int arr[] = { 1, 2, 1, INT_MAX, 3 };
    momo::stdish::multiset s(std::begin(arr), std::end(arr), std::greater<int>());

    ASSERT_SAME_TYPE(decltype(s), momo::stdish::multiset<int, std::greater<int> >);
    const int expected_s[] = { INT_MAX, 3, 2, 1, 1 };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
  }

  {
    const int arr[] = { 1, 2, 1, INT_MAX, 3 };
    momo::stdish::multiset s(std::begin(arr), std::end(arr), std::greater<int>(),
                    test_allocator<int>(0, 42));

    ASSERT_SAME_TYPE(
        decltype(s),
        momo::stdish::multiset<int, std::greater<int>, test_allocator<int> >);
    const int expected_s[] = { INT_MAX, 3, 2, 1, 1 };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
    assert(s.get_allocator().get_id() == 42);
  }

  {
    momo::stdish::multiset<long> source;
    momo::stdish::multiset s(source);
    ASSERT_SAME_TYPE(decltype(s), momo::stdish::multiset<long>);
    assert(s.size() == 0);
  }

  {
    momo::stdish::multiset<long> source;
    momo::stdish::multiset s{ source };  // braces instead of parens
    ASSERT_SAME_TYPE(decltype(s), momo::stdish::multiset<long>);
    assert(s.size() == 0);
  }

#if !defined(TEST_GCC) && !defined(TEST_CLANG)
  {
    momo::stdish::multiset<long> source;
    momo::stdish::multiset s(source, momo::stdish::multiset<long>::allocator_type());
    ASSERT_SAME_TYPE(decltype(s), momo::stdish::multiset<long>);
    assert(s.size() == 0);
  }
#endif

#if !defined(TEST_GCC)
  {
    momo::stdish::multiset s{ 1, 2, 1, INT_MAX, 3 };

    ASSERT_SAME_TYPE(decltype(s), momo::stdish::multiset<int>);
    const int expected_s[] = { 1, 1, 2, 3, INT_MAX };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
  }
#endif

  {
    momo::stdish::multiset s({ 1, 2, 1, INT_MAX, 3 }, std::greater<int>());

    ASSERT_SAME_TYPE(decltype(s), momo::stdish::multiset<int, std::greater<int> >);
    const int expected_s[] = { INT_MAX, 3, 2, 1, 1 };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
  }

  {
    momo::stdish::multiset s({ 1, 2, 1, INT_MAX, 3 }, std::greater<int>(),
                    test_allocator<int>(0, 43));

    ASSERT_SAME_TYPE(
        decltype(s),
        momo::stdish::multiset<int, std::greater<int>, test_allocator<int> >);
    const int expected_s[] = { INT_MAX, 3, 2, 1, 1 };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
    assert(s.get_allocator().get_id() == 43);
  }

  {
    const int arr[] = { 1, 2, 1, INT_MAX, 3 };
    momo::stdish::multiset s(std::begin(arr), std::end(arr), test_allocator<int>(0, 44));

    ASSERT_SAME_TYPE(decltype(s),
                     momo::stdish::multiset<int, std::less<int>, test_allocator<int> >);
    const int expected_s[] = { 1, 1, 2, 3, INT_MAX };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
    assert(s.get_allocator().get_id() == 44);
  }

  {
    momo::stdish::multiset s({ 1, 2, 1, INT_MAX, 3 }, test_allocator<int>(0, 45));

    ASSERT_SAME_TYPE(decltype(s),
                     momo::stdish::multiset<int, std::less<int>, test_allocator<int> >);
    const int expected_s[] = { 1, 1, 2, 3, INT_MAX };
    assert(std::equal(s.begin(), s.end(), std::begin(expected_s),
                      std::end(expected_s)));
    assert(s.get_allocator().get_id() == 45);
  }

#if !defined(TEST_GCC)
  {
    NotAnAllocator a;
    momo::stdish::multiset s{ a }; // multiset(initializer_list<NotAnAllocator>)
    ASSERT_SAME_TYPE(decltype(s), momo::stdish::multiset<NotAnAllocator>);
    assert(s.size() == 1);
  }

  {
    momo::stdish::multiset<long> source;
    momo::stdish::multiset s{ source, source }; // multiset(initializer_list<multiset<long>>)
    ASSERT_SAME_TYPE(decltype(s), momo::stdish::multiset<momo::stdish::multiset<long> >);
    assert(s.size() == 2);
  }

  {
    NotAnAllocator a;
    momo::stdish::multiset s{ a, a }; // multiset(initializer_list<NotAnAllocator>)
    ASSERT_SAME_TYPE(decltype(s), momo::stdish::multiset<NotAnAllocator>);
    assert(s.size() == 2);
  }
#endif

  {
    int source[3] = { 3, 4, 5 };
    momo::stdish::multiset s(source, source + 3); // multiset(InputIterator, InputIterator)
    ASSERT_SAME_TYPE(decltype(s), momo::stdish::multiset<int>);
    assert(s.size() == 3);
  }

#if !defined(TEST_GCC)
  {
    int source[3] = { 3, 4, 5 };
    momo::stdish::multiset s{ source, source + 3 }; // multiset(initializer_list<int*>)
    ASSERT_SAME_TYPE(decltype(s), momo::stdish::multiset<int *>);
    assert(s.size() == 2);
  }
#endif

#if TEST_STD_VER >= 23
    {
      using Range = std::array<int, 0>;
      using Comp = std::greater<int>;
      using DefaultComp = std::less<int>;
      using Alloc = test_allocator<int>;

      { // (from_range, range)
        momo::stdish::multiset c(std::from_range, Range());
        static_assert(std::is_same_v<decltype(c), momo::stdish::multiset<int>>);
      }

      { // (from_range, range, comp)
        momo::stdish::multiset c(std::from_range, Range(), Comp());
        static_assert(std::is_same_v<decltype(c), momo::stdish::multiset<int, Comp>>);
      }

      { // (from_range, range, comp, alloc)
        momo::stdish::multiset c(std::from_range, Range(), Comp(), Alloc());
        static_assert(std::is_same_v<decltype(c), momo::stdish::multiset<int, Comp, Alloc>>);
      }

      { // (from_range, range, alloc)
        momo::stdish::multiset c(std::from_range, Range(), Alloc());
        static_assert(std::is_same_v<decltype(c), momo::stdish::multiset<int, DefaultComp, Alloc>>);
      }
    }
#endif

//#if MOMO_VERSION_MAJOR > 3
#if !(defined(TEST_MSVC) && _MSC_VER < 1930) && !(defined(TEST_GCC) && __GNUC__ < 11)
  AssociativeContainerDeductionGuidesSfinaeAway<std::multiset, std::multiset<int>>();
#endif
  AssociativeContainerDeductionGuidesSfinaeAway<momo::stdish::multiset, momo::stdish::multiset<int>>();
//#endif

  return 0;
}

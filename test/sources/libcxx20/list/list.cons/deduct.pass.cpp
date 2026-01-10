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

// <list>
// UNSUPPORTED: c++03, c++11, c++14

// template <class InputIterator, class Allocator = allocator<typename iterator_traits<InputIterator>::value_type>>
//    list(InputIterator, InputIterator, Allocator = Allocator())
//    -> list<typename iterator_traits<InputIterator>::value_type, Allocator>;
//
// template<ranges::input_range R, class Allocator = allocator<ranges::range_value_t<R>>>
//   list(from_range_t, R&&, Allocator = Allocator())
//     -> list<ranges::range_value_t<R>, Allocator>; // C++23

struct A {};

int main(int, char**) {
  //  Test the explicit deduction guides
  {
    const int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    LIBCXX_TEST_CLASS lst(std::begin(arr), std::end(arr));

    static_assert(std::is_same_v<decltype(lst), LIBCXX_TEST_CLASS<int>>, "");
    assert(std::equal(lst.begin(), lst.end(), std::begin(arr), std::end(arr)));
  }

  {
    const long arr[] = {INT_MAX, 1L, 2L, 3L};
    LIBCXX_TEST_CLASS lst(std::begin(arr), std::end(arr), std::allocator<long>());
    static_assert(std::is_same_v<decltype(lst)::value_type, long>, "");
    assert(lst.size() == 4);
    auto it = lst.begin();
    assert(*it++ == INT_MAX);
    assert(*it++ == 1L);
    assert(*it++ == 2L);
  }

  //  Test the implicit deduction guides

  {
    //  We don't expect this one to work.
    //  LIBCXX_TEST_CLASS lst(std::allocator<int>()); // list (allocator &)
  }

  {
    LIBCXX_TEST_CLASS lst(1, A{}); // list (size_type, T)
    static_assert(std::is_same_v<decltype(lst)::value_type, A>, "");
    static_assert(std::is_same_v<decltype(lst)::allocator_type, std::allocator<A>>, "");
    assert(lst.size() == 1);
  }

  {
    LIBCXX_TEST_CLASS lst(1, A{}, test_allocator<A>()); // list (size_type, T, allocator)
    static_assert(std::is_same_v<decltype(lst)::value_type, A>, "");
    static_assert(std::is_same_v<decltype(lst)::allocator_type, test_allocator<A>>, "");
    assert(lst.size() == 1);
  }

  {
    LIBCXX_TEST_CLASS lst{1U, 2U, 3U, 4U, 5U}; // list(initializer-list)
    static_assert(std::is_same_v<decltype(lst)::value_type, unsigned>, "");
    assert(lst.size() == 5);
    auto it = lst.begin();
    std::advance(it, 2);
    assert(*it == 3U);
  }

  {
    LIBCXX_TEST_CLASS lst({1.0, 2.0, 3.0, 4.0}, test_allocator<double>()); // list(initializer-list, allocator)
    static_assert(std::is_same_v<decltype(lst)::value_type, double>, "");
    static_assert(std::is_same_v<decltype(lst)::allocator_type, test_allocator<double>>, "");
    assert(lst.size() == 4);
    auto it = lst.begin();
    std::advance(it, 3);
    assert(*it == 4.0);
  }

  {
    LIBCXX_TEST_CLASS<long double> source;
    LIBCXX_TEST_CLASS lst(source); // list(list &)
    static_assert(std::is_same_v<decltype(lst)::value_type, long double>, "");
    static_assert(std::is_same_v<decltype(lst)::allocator_type, std::allocator<long double>>, "");
    assert(lst.size() == 0);
  }

  {
    typedef test_allocator<short> Alloc;
    typedef test_allocator<int> ConvertibleToAlloc;

    {
      LIBCXX_TEST_CLASS<short, Alloc> source;
      LIBCXX_TEST_CLASS lst(source, Alloc(2));
      static_assert(std::is_same_v<decltype(lst), decltype(source)>);
    }

    {
      LIBCXX_TEST_CLASS<short, Alloc> source;
      LIBCXX_TEST_CLASS lst(source, ConvertibleToAlloc(2));
      static_assert(std::is_same_v<decltype(lst), decltype(source)>);
    }

    {
      LIBCXX_TEST_CLASS<short, Alloc> source;
      LIBCXX_TEST_CLASS lst(std::move(source), Alloc(2));
      static_assert(std::is_same_v<decltype(lst), decltype(source)>);
    }

    {
      LIBCXX_TEST_CLASS<short, Alloc> source;
      LIBCXX_TEST_CLASS lst(std::move(source), ConvertibleToAlloc(2));
      static_assert(std::is_same_v<decltype(lst), decltype(source)>);
    }
  }

#if TEST_STD_VER >= 23
  {
    {
      LIBCXX_TEST_CLASS c(std::from_range, std::array<int, 0>());
      static_assert(std::is_same_v<decltype(c), LIBCXX_TEST_CLASS<int>>);
    }

    {
      using Alloc = test_allocator<int>;
      LIBCXX_TEST_CLASS c(std::from_range, std::array<int, 0>(), Alloc());
      static_assert(std::is_same_v<decltype(c), LIBCXX_TEST_CLASS<int, Alloc>>);
    }
  }
#endif

  SequenceContainerDeductionGuidesSfinaeAway<LIBCXX_TEST_CLASS, LIBCXX_TEST_CLASS<int>>();

  return 0;
}

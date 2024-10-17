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
// UNSUPPORTED: c++03, c++11, c++14

// template <class InputIterator, class Allocator = allocator<typename iterator_traits<InputIterator>::value_type>>
//    vector(InputIterator, InputIterator, Allocator = Allocator())
//    -> vector<typename iterator_traits<InputIterator>::value_type, Allocator>;
//
// template<ranges::input_range R, class Allocator = allocator<ranges::range_value_t<R>>>
//   vector(from_range_t, R&&, Allocator = Allocator())
//     -> vector<ranges::range_value_t<R>, Allocator>; // C++23

struct A {};

TEST_CONSTEXPR_CXX20 bool tests() {

//  Test the explicit deduction guides
    {
    const int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    momo::stdish::vector vec(std::begin(arr), std::end(arr));

    static_assert(std::is_same_v<decltype(vec), momo::stdish::vector<int>>, "");
    assert(std::equal(vec.begin(), vec.end(), std::begin(arr), std::end(arr)));
    }

    {
    const long arr[] = {INT_MAX, 1L, 2L, 3L };
    momo::stdish::vector vec(std::begin(arr), std::end(arr), std::allocator<long>());
    static_assert(std::is_same_v<decltype(vec)::value_type, long>, "");
    assert(vec.size() == 4);
    assert(vec[0] == INT_MAX);
    assert(vec[1] == 1L);
    assert(vec[2] == 2L);
    }

//  Test the implicit deduction guides

    {
//  We don't expect this one to work.
//  momo::stdish::vector vec(std::allocator<int>()); // vector (allocator &)
    }

    {
    momo::stdish::vector vec(1, A{}); // vector (size_type, T)
    static_assert(std::is_same_v<decltype(vec)::value_type, A>, "");
    static_assert(std::is_same_v<decltype(vec)::allocator_type, std::allocator<A>>, "");
    assert(vec.size() == 1);
    }

    {
    momo::stdish::vector vec(1, A{}, test_allocator<A>()); // vector (size_type, T, allocator)
    static_assert(std::is_same_v<decltype(vec)::value_type, A>, "");
    static_assert(std::is_same_v<decltype(vec)::allocator_type, test_allocator<A>>, "");
    assert(vec.size() == 1);
    }

    {
    momo::stdish::vector vec{1U, 2U, 3U, 4U, 5U}; // vector(initializer-list)
    static_assert(std::is_same_v<decltype(vec)::value_type, unsigned>, "");
    assert(vec.size() == 5);
    assert(vec[2] == 3U);
    }

    {
    momo::stdish::vector vec({1.0, 2.0, 3.0, 4.0}, test_allocator<double>()); // vector(initializer-list, allocator)
    static_assert(std::is_same_v<decltype(vec)::value_type, double>, "");
    static_assert(std::is_same_v<decltype(vec)::allocator_type, test_allocator<double>>, "");
    assert(vec.size() == 4);
    assert(vec[3] == 4.0);
    }

    {
    momo::stdish::vector<long double> source;
    momo::stdish::vector vec(source); // vector(vector &)
    static_assert(std::is_same_v<decltype(vec)::value_type, long double>, "");
    static_assert(std::is_same_v<decltype(vec)::allocator_type, std::allocator<long double>>, "");
    assert(vec.size() == 0);
    }

#if TEST_STD_VER >= 23
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    {
      {
        momo::stdish::vector c(std::from_range, std::array<int, 0>());
        static_assert(std::is_same_v<decltype(c), momo::stdish::vector<int>>);
      }

      {
        using Alloc = test_allocator<int>;
        momo::stdish::vector c(std::from_range, std::array<int, 0>(), Alloc());
        static_assert(std::is_same_v<decltype(c), momo::stdish::vector<int, Alloc>>);
      }
    }
#endif
#endif

//  A couple of vector<bool> tests, too!
    {
    momo::stdish::vector vec(3, true); // vector(initializer-list)
    static_assert(std::is_same_v<decltype(vec)::value_type, bool>, "");
    static_assert(std::is_same_v<decltype(vec)::allocator_type, std::allocator<bool>>, "");
    assert(vec.size() == 3);
    assert(vec[0] && vec[1] && vec[2]);
    }

    {
    momo::stdish::vector<bool> source;
    momo::stdish::vector vec(source); // vector(vector &)
    static_assert(std::is_same_v<decltype(vec)::value_type, bool>, "");
    static_assert(std::is_same_v<decltype(vec)::allocator_type, std::allocator<bool>>, "");
    assert(vec.size() == 0);
    }

    {
        typedef test_allocator<short> Alloc;
        typedef test_allocator<int> ConvertibleToAlloc;

        {
        momo::stdish::vector<short, Alloc> source;
        momo::stdish::vector vec(source, Alloc(2));
        static_assert(std::is_same_v<decltype(vec), decltype(source)>);
        }

        {
        momo::stdish::vector<short, Alloc> source;
        momo::stdish::vector vec(source, ConvertibleToAlloc(2));
        static_assert(std::is_same_v<decltype(vec), decltype(source)>);
        }

        {
        momo::stdish::vector<short, Alloc> source;
        momo::stdish::vector vec(std::move(source), Alloc(2));
        static_assert(std::is_same_v<decltype(vec), decltype(source)>);
        }

        {
        momo::stdish::vector<short, Alloc> source;
        momo::stdish::vector vec(std::move(source), ConvertibleToAlloc(2));
        static_assert(std::is_same_v<decltype(vec), decltype(source)>);
        }
    }

#if !(defined(TEST_GCC) && __GNUC__ < 11)
#if MOMO_VERSION_MAJOR > 3
    SequenceContainerDeductionGuidesSfinaeAway<std::vector, std::vector<int>>();
#endif
#endif

    return true;
}

int main(int, char**) {
    tests();
#if TEST_STD_VER > 17
    //static_assert(tests());
#endif
    return 0;
}

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

// <map>
// UNSUPPORTED: c++03, c++11, c++14

// template<class InputIterator,
//          class Compare = less<iter-value-type<InputIterator>>,
//          class Allocator = allocator<iter-value-type<InputIterator>>>
// multimap(InputIterator, InputIterator,
//          Compare = Compare(), Allocator = Allocator())
//   -> multimap<iter-value-type<InputIterator>, Compare, Allocator>;
// template<class Key, class Compare = less<Key>, class Allocator = allocator<Key>>
// multimap(initializer_list<Key>, Compare = Compare(), Allocator = Allocator())
//   -> multimap<Key, Compare, Allocator>;
// template<class InputIterator, class Allocator>
// multimap(InputIterator, InputIterator, Allocator)
//   -> multimap<iter-value-type<InputIterator>, less<iter-value-type<InputIterator>>, Allocator>;
// template<class Key, class Allocator>
// multimap(initializer_list<Key>, Allocator)
//   -> multimap<Key, less<Key>, Allocator>;
//
// template<ranges::input_range R, class Compare = less<range-key-type<R>>,
//           class Allocator = allocator<range-to-alloc-type<R>>>
//   multimap(from_range_t, R&&, Compare = Compare(), Allocator = Allocator())
//     -> multimap<range-key-type<R>, range-mapped-type<R>, Compare, Allocator>; // C++23
//
// template<ranges::input_range R, class Allocator>
//   multimap(from_range_t, R&&, Allocator)
//     -> multimap<range-key-type<R>, range-mapped-type<R>, less<range-key-type<R>>, Allocator>; // C++23

using P = std::pair<int, long>;
using PC = std::pair<const int, long>;

int main(int, char**)
{
    {
    const P arr[] = { {1,1L}, {2,2L}, {1,1L}, {INT_MAX,1L}, {3,1L} };
    momo::stdish::multimap m(std::begin(arr), std::end(arr));

    ASSERT_SAME_TYPE(decltype(m), momo::stdish::multimap<int, long>);
    const PC expected_m[] = { {1,1L}, {1,1L}, {2,2L}, {3,1L}, {INT_MAX,1L} };
    assert(std::equal(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    }

    {
    const P arr[] = { {1,1L}, {2,2L}, {1,1L}, {INT_MAX,1L}, {3,1L} };
    momo::stdish::multimap m(std::begin(arr), std::end(arr), std::greater<int>());

    ASSERT_SAME_TYPE(decltype(m), momo::stdish::multimap<int, long, std::greater<int>>);
    const PC expected_m[] = { {INT_MAX,1L}, {3,1L}, {2,2L}, {1,1L}, {1,1L} };
    assert(std::equal(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    }

    {
    const P arr[] = { {1,1L}, {2,2L}, {1,1L}, {INT_MAX,1L}, {3,1L} };
    momo::stdish::multimap m(std::begin(arr), std::end(arr), std::greater<int>(), test_allocator<PC>(0, 42));

    ASSERT_SAME_TYPE(decltype(m), momo::stdish::multimap<int, long, std::greater<int>, test_allocator<PC>>);
    const PC expected_m[] = { {INT_MAX,1L}, {3,1L}, {2,2L}, {1,1L}, {1,1L} };
    assert(std::equal(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    assert(m.get_allocator().get_id() == 42);
    }

    {
    momo::stdish::multimap<int, long> source;
    momo::stdish::multimap m(source);
    ASSERT_SAME_TYPE(decltype(m), decltype(source));
    assert(m.size() == 0);
    }

    {
    momo::stdish::multimap<int, long> source;
    momo::stdish::multimap m{source};  // braces instead of parens
    ASSERT_SAME_TYPE(decltype(m), decltype(source));
    assert(m.size() == 0);
    }

#if !defined(TEST_GCC) && !defined(TEST_CLANG)
    {
    momo::stdish::multimap<int, long> source;
    momo::stdish::multimap m(source, momo::stdish::multimap<int, long>::allocator_type());
    ASSERT_SAME_TYPE(decltype(m), decltype(source));
    assert(m.size() == 0);
    }
#endif

#if !(defined(TEST_GCC) && __GNUC__ < 13)
    {
    momo::stdish::multimap m{ P{1,1L}, P{2,2L}, P{1,1L}, P{INT_MAX,1L}, P{3,1L} };

    ASSERT_SAME_TYPE(decltype(m), momo::stdish::multimap<int, long>);
    const PC expected_m[] = { {1,1L}, {1,1L}, {2,2L}, {3,1L}, {INT_MAX,1L} };
    assert(std::equal(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    }
#endif

    {
    momo::stdish::multimap m({ P{1,1L}, P{2,2L}, P{1,1L}, P{INT_MAX,1L}, P{3,1L} }, std::greater<int>());

    ASSERT_SAME_TYPE(decltype(m), momo::stdish::multimap<int, long, std::greater<int>>);
    const PC expected_m[] = { {INT_MAX,1L}, {3,1L}, {2,2L}, {1,1L}, {1,1L} };
    assert(std::equal(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    }

    {
    momo::stdish::multimap m({ P{1,1L}, P{2,2L}, P{1,1L}, P{INT_MAX,1L}, P{3,1L} }, std::greater<int>(), test_allocator<PC>(0, 43));

    ASSERT_SAME_TYPE(decltype(m), momo::stdish::multimap<int, long, std::greater<int>, test_allocator<PC>>);
    const PC expected_m[] = { {INT_MAX,1L}, {3,1L}, {2,2L}, {1,1L}, {1,1L} };
    assert(std::equal(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    assert(m.get_allocator().get_id() == 43);
    }

    {
    const P arr[] = { {1,1L}, {2,2L}, {1,1L}, {INT_MAX,1L}, {3,1L} };
    momo::stdish::multimap m(std::begin(arr), std::end(arr), test_allocator<PC>(0, 44));

    ASSERT_SAME_TYPE(decltype(m), momo::stdish::multimap<int, long, std::less<int>, test_allocator<PC>>);
    const PC expected_m[] = { {1,1L}, {1,1L}, {2,2L}, {3,1L}, {INT_MAX,1L} };
    assert(std::equal(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    assert(m.get_allocator().get_id() == 44);
    }

    {
    momo::stdish::multimap m({ P{1,1L}, P{2,2L}, P{1,1L}, P{INT_MAX,1L}, P{3,1L} }, test_allocator<PC>(0, 45));

    ASSERT_SAME_TYPE(decltype(m), momo::stdish::multimap<int, long, std::less<int>, test_allocator<PC>>);
    const PC expected_m[] = { {1,1L}, {1,1L}, {2,2L}, {3,1L}, {INT_MAX,1L} };
    assert(std::equal(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    assert(m.get_allocator().get_id() == 45);
    }

#if !(defined(TEST_GCC) && __GNUC__ < 13)
    {
    // Examples from LWG3025
    momo::stdish::multimap m{std::pair{1, 1}, {2, 2}, {3, 3}};
    ASSERT_SAME_TYPE(decltype(m), momo::stdish::multimap<int, int>);

    momo::stdish::multimap m2{m.begin(), m.end()};
    ASSERT_SAME_TYPE(decltype(m2), momo::stdish::multimap<int, int>);
    }
#endif

    {
    // Examples from LWG3531
    momo::stdish::multimap m1{{std::pair{1, 2}, {3, 4}}, std::less<int>()};
    ASSERT_SAME_TYPE(decltype(m1), momo::stdish::multimap<int, int>);

    using value_type = std::pair<const int, int>;
    momo::stdish::multimap m2{{value_type{1, 2}, {3, 4}}, std::less<int>()};
    ASSERT_SAME_TYPE(decltype(m2), momo::stdish::multimap<int, int>);
    }

#if TEST_STD_VER >= 23
    {
      using Range = std::array<P, 0>;
      using Comp = std::greater<int>;
      using DefaultComp = std::less<int>;
      using Alloc = test_allocator<PC>;

      { // (from_range, range)
        momo::stdish::multimap c(std::from_range, Range());
        static_assert(std::is_same_v<decltype(c), momo::stdish::multimap<int, long>>);
      }

      { // (from_range, range, comp)
        momo::stdish::multimap c(std::from_range, Range(), Comp());
        static_assert(std::is_same_v<decltype(c), momo::stdish::multimap<int, long, Comp>>);
      }

      { // (from_range, range, comp, alloc)
        momo::stdish::multimap c(std::from_range, Range(), Comp(), Alloc());
        static_assert(std::is_same_v<decltype(c), momo::stdish::multimap<int, long, Comp, Alloc>>);
      }

      { // (from_range, range, alloc)
        momo::stdish::multimap c(std::from_range, Range(), Alloc());
        static_assert(std::is_same_v<decltype(c), momo::stdish::multimap<int, long, DefaultComp, Alloc>>);
      }
    }
#endif

    //AssociativeContainerDeductionGuidesSfinaeAway<std::multimap, std::multimap<int, long>>();
    AssociativeContainerDeductionGuidesSfinaeAway<momo::stdish::multimap, momo::stdish::multimap<int, long>>();

    return 0;
}

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
// map(InputIterator, InputIterator,
//          Compare = Compare(), Allocator = Allocator())
//   -> map<iter-value-type<InputIterator>, Compare, Allocator>;
// template<class Key, class Compare = less<Key>, class Allocator = allocator<Key>>
// map(initializer_list<Key>, Compare = Compare(), Allocator = Allocator())
//   -> map<Key, Compare, Allocator>;
// template<class InputIterator, class Allocator>
// map(InputIterator, InputIterator, Allocator)
//   -> map<iter-value-type<InputIterator>, less<iter-value-type<InputIterator>>, Allocator>;
// template<class Key, class Allocator>
// map(initializer_list<Key>, Allocator)
//   -> map<Key, less<Key>, Allocator>;
//
// template<ranges::input_range R, class Compare = less<range-key-type<R>,
//          class Allocator = allocator<range-to-alloc-type<R>>>
//   map(from_range_t, R&&, Compare = Compare(), Allocator = Allocator())
//     -> map<range-key-type<R>, range-mapped-type<R>, Compare, Allocator>; // C++23
//
// template<ranges::input_range R, class Allocator>
//   map(from_range_t, R&&, Allocator)
//     -> map<range-key-type<R>, range-mapped-type<R>, less<range-key-type<R>>, Allocator>; // C++23

using P = std::pair<int, long>;
using PC = std::pair<const int, long>;

int main(int, char**)
{
    {
    const P arr[] = { {1,1L}, {2,2L}, {1,1L}, {INT_MAX,1L}, {3,1L} };
    momo::stdish::map m(std::begin(arr), std::end(arr));

    ASSERT_SAME_TYPE(decltype(m), momo::stdish::map<int, long>);
    const PC expected_m[] = { {1,1L}, {2,2L}, {3,1L}, {INT_MAX,1L} };
    assert(std::equal(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    }

    {
    const P arr[] = { {1,1L}, {2,2L}, {1,1L}, {INT_MAX,1L}, {3,1L} };
    momo::stdish::map m(std::begin(arr), std::end(arr), std::greater<int>());

    ASSERT_SAME_TYPE(decltype(m), momo::stdish::map<int, long, std::greater<int>>);
    const PC expected_m[] = { {INT_MAX,1L}, {3,1L}, {2,2L}, {1,1L} };
    assert(std::equal(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    }

    {
    const P arr[] = { {1,1L}, {2,2L}, {1,1L}, {INT_MAX,1L}, {3,1L} };
    momo::stdish::map m(std::begin(arr), std::end(arr), std::greater<int>(), test_allocator<PC>(0, 42));

    ASSERT_SAME_TYPE(decltype(m), momo::stdish::map<int, long, std::greater<int>, test_allocator<PC>>);
    const PC expected_m[] = { {INT_MAX,1L}, {3,1L}, {2,2L}, {1,1L} };
    assert(std::equal(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    assert(m.get_allocator().get_id() == 42);
    }

    {
    momo::stdish::map<int, long> source;
    momo::stdish::map m(source);
    ASSERT_SAME_TYPE(decltype(m), decltype(source));
    assert(m.size() == 0);
    }

    {
    momo::stdish::map<int, long> source;
    momo::stdish::map m{source};  // braces instead of parens
    ASSERT_SAME_TYPE(decltype(m), decltype(source));
    assert(m.size() == 0);
    }

#if !defined(TEST_GCC) && !defined(TEST_CLANG)
    {
    momo::stdish::map<int, long> source;
    momo::stdish::map m(source, momo::stdish::map<int, long>::allocator_type());
    ASSERT_SAME_TYPE(decltype(m), decltype(source));
    assert(m.size() == 0);
    }
#endif

#if !(defined(TEST_GCC) && __GNUC__ < 13)
    {
    momo::stdish::map m{ P{1,1L}, P{2,2L}, P{1,1L}, P{INT_MAX,1L}, P{3,1L} };

    ASSERT_SAME_TYPE(decltype(m), momo::stdish::map<int, long>);
    const PC expected_m[] = { {1,1L}, {2,2L}, {3,1L}, {INT_MAX,1L} };
    assert(std::equal(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    }
#endif

    {
    momo::stdish::map m({ P{1,1L}, P{2,2L}, P{1,1L}, P{INT_MAX,1L}, P{3,1L} }, std::greater<int>());

    ASSERT_SAME_TYPE(decltype(m), momo::stdish::map<int, long, std::greater<int>>);
    const PC expected_m[] = { {INT_MAX,1L}, {3,1L}, {2,2L}, {1,1L} };
    assert(std::equal(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    }

    {
    momo::stdish::map m({ P{1,1L}, P{2,2L}, P{1,1L}, P{INT_MAX,1L}, P{3,1L} }, std::greater<int>(), test_allocator<PC>(0, 43));

    ASSERT_SAME_TYPE(decltype(m), momo::stdish::map<int, long, std::greater<int>, test_allocator<PC>>);
    const PC expected_m[] = { {INT_MAX,1L}, {3,1L}, {2,2L}, {1,1L} };
    assert(std::equal(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    assert(m.get_allocator().get_id() == 43);
    }

    {
    const P arr[] = { {1,1L}, {2,2L}, {1,1L}, {INT_MAX,1L}, {3,1L} };
    momo::stdish::map m(std::begin(arr), std::end(arr), test_allocator<PC>(0, 44));

    ASSERT_SAME_TYPE(decltype(m), momo::stdish::map<int, long, std::less<int>, test_allocator<PC>>);
    const PC expected_m[] = { {1,1L}, {2,2L}, {3,1L}, {INT_MAX,1L} };
    assert(std::equal(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    assert(m.get_allocator().get_id() == 44);
    }

    {
    momo::stdish::map m({ P{1,1L}, P{2,2L}, P{1,1L}, P{INT_MAX,1L}, P{3,1L} }, test_allocator<PC>(0, 45));

    ASSERT_SAME_TYPE(decltype(m), momo::stdish::map<int, long, std::less<int>, test_allocator<PC>>);
    const PC expected_m[] = { {1,1L}, {2,2L}, {3,1L}, {INT_MAX,1L} };
    assert(std::equal(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    assert(m.get_allocator().get_id() == 45);
    }

#if !(defined(TEST_GCC) && __GNUC__ < 13)
    {
    // Examples from LWG3025
    momo::stdish::map m{std::pair{1, 1}, {2, 2}, {3, 3}};
    ASSERT_SAME_TYPE(decltype(m), momo::stdish::map<int, int>);

    momo::stdish::map m2{m.begin(), m.end()};
    ASSERT_SAME_TYPE(decltype(m2), momo::stdish::map<int, int>);
    }
#endif

    {
    // Examples from LWG3531
    momo::stdish::map m1{{std::pair{1, 2}, {3, 4}}, std::less<int>()};
    ASSERT_SAME_TYPE(decltype(m1), momo::stdish::map<int, int>);

    using value_type = std::pair<const int, int>;
    momo::stdish::map m2{{value_type{1, 2}, {3, 4}}, std::less<int>()};
    ASSERT_SAME_TYPE(decltype(m2), momo::stdish::map<int, int>);
    }

#if TEST_STD_VER >= 23
    {
      using Range = std::array<P, 0>;
      using Comp = std::greater<int>;
      using DefaultComp = std::less<int>;
      using Alloc = test_allocator<PC>;

      { // (from_range, range)
        momo::stdish::map c(std::from_range, Range());
        static_assert(std::is_same_v<decltype(c), momo::stdish::map<int, long>>);
      }

      { // (from_range, range, comp)
        momo::stdish::map c(std::from_range, Range(), Comp());
        static_assert(std::is_same_v<decltype(c), momo::stdish::map<int, long, Comp>>);
      }

      { // (from_range, range, comp, alloc)
        momo::stdish::map c(std::from_range, Range(), Comp(), Alloc());
        static_assert(std::is_same_v<decltype(c), momo::stdish::map<int, long, Comp, Alloc>>);
      }

      { // (from_range, range, alloc)
        momo::stdish::map c(std::from_range, Range(), Alloc());
        static_assert(std::is_same_v<decltype(c), momo::stdish::map<int, long, DefaultComp, Alloc>>);
      }
    }
#endif

    //AssociativeContainerDeductionGuidesSfinaeAway<std::map, std::map<int, long>>();
    AssociativeContainerDeductionGuidesSfinaeAway<momo::stdish::map, momo::stdish::map<int, long>>();

    return 0;
}

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

// <unordered_map>
// UNSUPPORTED: c++03, c++11, c++14

// template<class InputIterator,
//          class Hash = hash<iter-key-type<InputIterator>>,
//          class Pred = equal_to<iter-key-type<InputIterator>>,
//          class Allocator = allocator<iter-to-alloc-type<InputIterator>>>
// unordered_map(InputIterator, InputIterator, typename see below::size_type = see below,
//               Hash = Hash(), Pred = Pred(), Allocator = Allocator())
//   -> unordered_map<iter-key-type<InputIterator>, iter-mapped-type<InputIterator>, Hash, Pred,
//                    Allocator>;
//
// template<class Key, class T, class Hash = hash<Key>,
//          class Pred = equal_to<Key>, class Allocator = allocator<pair<const Key, T>>>
// unordered_map(initializer_list<pair<Key, T>>,
//               typename see below::size_type = see below, Hash = Hash(),
//               Pred = Pred(), Allocator = Allocator())
//   -> unordered_map<Key, T, Hash, Pred, Allocator>;
//
// template<class InputIterator, class Allocator>
// unordered_map(InputIterator, InputIterator, typename see below::size_type, Allocator)
//   -> unordered_map<iter-key-type<InputIterator>, iter-mapped-type<InputIterator>,
//                    hash<iter-key-type<InputIterator>>,
//                    equal_to<iter-key-type<InputIterator>>, Allocator>;
//
// template<class InputIterator, class Allocator>
// unordered_map(InputIterator, InputIterator, Allocator)
//   -> unordered_map<iter-key-type<InputIterator>, iter-mapped-type<InputIterator>,
//                    hash<iter-key-type<InputIterator>>,
//                    equal_to<iter-key-type<InputIterator>>, Allocator>;
//
// template<class InputIterator, class Hash, class Allocator>
// unordered_map(InputIterator, InputIterator, typename see below::size_type, Hash, Allocator)
//   -> unordered_map<iter-key-type<InputIterator>, iter-mapped-type<InputIterator>, Hash,
//                    equal_to<iter-key-type<InputIterator>>, Allocator>;
//
// template<class Key, class T, class Allocator>
// unordered_map(initializer_list<pair<Key, T>>, typename see below::size_type, Allocator)
//   -> unordered_map<Key, T, hash<Key>, equal_to<Key>, Allocator>;
//
// template<class Key, class T, class Allocator>
// unordered_map(initializer_list<pair<Key, T>>, Allocator)
//   -> unordered_map<Key, T, hash<Key>, equal_to<Key>, Allocator>;
//
// template<class Key, class T, class Hash, class Allocator>
// unordered_map(initializer_list<pair<Key, T>>, typename see below::size_type, Hash,
//               Allocator)
//   -> unordered_map<Key, T, Hash, equal_to<Key>, Allocator>;
//
// template<ranges::input_range R, class Hash = hash<range-key-type<R>>,
//           class Pred = equal_to<range-key-type<R>>,
//           class Allocator = allocator<range-to-alloc-type<R>>>
//   unordered_map(from_range_t, R&&, typename see below::size_type = see below,
//                 Hash = Hash(), Pred = Pred(), Allocator = Allocator())
//     -> unordered_map<range-key-type<R>, range-mapped-type<R>, Hash, Pred, Allocator>; // C++23
//
// template<ranges::input_range R, class Allocator>
//   unordered_map(from_range_t, R&&, typename see below::size_type, Allocator)
//     -> unordered_map<range-key-type<R>, range-mapped-type<R>, hash<range-key-type<R>>,
//                       equal_to<range-key-type<R>>, Allocator>;   // C++23
//
// template<ranges::input_range R, class Allocator>
//   unordered_map(from_range_t, R&&, Allocator)
//     -> unordered_map<range-key-type<R>, range-mapped-type<R>, hash<range-key-type<R>>,
//                       equal_to<range-key-type<R>>, Allocator>;   // C++23
//
// template<ranges::input_range R, class Hash, class Allocator>
//   unordered_map(from_range_t, R&&, typename see below::size_type, Hash, Allocator)
//     -> unordered_map<range-key-type<R>, range-mapped-type<R>, Hash,
//                       equal_to<range-key-type<R>>, Allocator>;   // C++23

using P = std::pair<int, long>;
using PC = std::pair<const int, long>;

int main(int, char**)
{
    const PC expected_m[] = { {1,1}, {2,2}, {3,1}, {INT_MAX,1} };

    {
    const P arr[] = { {1,1}, {2,2}, {1,1}, {INT_MAX,1}, {3,1} };
    momo::stdish::unordered_map m(std::begin(arr), std::end(arr));
    ASSERT_SAME_TYPE(decltype(m), momo::stdish::unordered_map<int, long>);
    assert(std::is_permutation(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    }

    {
    const P arr[] = { {1,1}, {2,2}, {1,1}, {INT_MAX,1}, {3,1} };
    momo::stdish::unordered_map m(std::begin(arr), std::end(arr), 42);
    ASSERT_SAME_TYPE(decltype(m), momo::stdish::unordered_map<int, long>);
    assert(std::is_permutation(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    }

    {
    const P arr[] = { {1,1}, {2,2}, {1,1}, {INT_MAX,1}, {3,1} };
    momo::stdish::unordered_map m(std::begin(arr), std::end(arr), 42, std::hash<long long>());
    ASSERT_SAME_TYPE(decltype(m), momo::stdish::unordered_map<int, long, std::hash<long long>, std::equal_to<int>>);
    assert(std::is_permutation(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    }

    {
    const P arr[] = { {1,1}, {2,2}, {1,1}, {INT_MAX,1}, {3,1} };
    momo::stdish::unordered_map m(std::begin(arr), std::end(arr), 42, std::hash<long long>(), std::equal_to<>());
    ASSERT_SAME_TYPE(decltype(m), momo::stdish::unordered_map<int, long, std::hash<long long>, std::equal_to<>>);
    assert(std::is_permutation(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    }

    {
    const P arr[] = { {1,1}, {2,2}, {1,1}, {INT_MAX,1}, {3,1} };
    momo::stdish::unordered_map m(std::begin(arr), std::end(arr), 42, std::hash<long long>(), std::equal_to<>(), test_allocator<PC>(0, 41));
    ASSERT_SAME_TYPE(decltype(m), momo::stdish::unordered_map<int, long, std::hash<long long>, std::equal_to<>, test_allocator<PC>>);
    assert(std::is_permutation(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    assert(m.get_allocator().get_id() == 41);
    }

    {
    momo::stdish::unordered_map<int, long> source;
    momo::stdish::unordered_map m(source);
    ASSERT_SAME_TYPE(decltype(m), decltype(source));
    assert(m.size() == 0);
    }

    {
    momo::stdish::unordered_map<int, long> source;
    momo::stdish::unordered_map m{source};  // braces instead of parens
    ASSERT_SAME_TYPE(decltype(m), decltype(source));
    assert(m.size() == 0);
    }

    {
    momo::stdish::unordered_map<int, long, std::hash<long long>, std::equal_to<>, test_allocator<PC>> source;
    test_allocator<PC> a(0, 42);
    momo::stdish::unordered_map m(source, a);
    ASSERT_SAME_TYPE(decltype(m), decltype(source));
    assert(m.get_allocator().get_id() == 42);
    assert(m.size() == 0);
    }

    {
    momo::stdish::unordered_map<int, long, std::hash<long long>, std::equal_to<>, test_allocator<PC>> source;
    test_allocator<PC> a(0, 43);
    momo::stdish::unordered_map m{source, a};  // braces instead of parens
    ASSERT_SAME_TYPE(decltype(m), decltype(source));
    assert(m.get_allocator().get_id() == 43);
    assert(m.size() == 0);
    }

    {
    momo::stdish::unordered_map m { P{1,1L}, P{2,2L}, P{1,1L}, P{INT_MAX,1L}, P{3,1L} };
    ASSERT_SAME_TYPE(decltype(m), momo::stdish::unordered_map<int, long>);
    assert(std::is_permutation(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    }

    {
    momo::stdish::unordered_map m({ P{1,1L}, P{2,2L}, P{1,1L}, P{INT_MAX,1L}, P{3,1L} }, 42);
    ASSERT_SAME_TYPE(decltype(m), momo::stdish::unordered_map<int, long>);
    assert(std::is_permutation(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    }

    {
    momo::stdish::unordered_map m({ P{1,1L}, P{2,2L}, P{1,1L}, P{INT_MAX,1L}, P{3,1L} }, 42, std::hash<long long>());
    ASSERT_SAME_TYPE(decltype(m), momo::stdish::unordered_map<int, long, std::hash<long long>>);
    assert(std::is_permutation(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    }

    {
    momo::stdish::unordered_map m({ P{1,1L}, P{2,2L}, P{1,1L}, P{INT_MAX,1L}, P{3,1L} }, 42, std::hash<long long>(), std::equal_to<>());
    ASSERT_SAME_TYPE(decltype(m), momo::stdish::unordered_map<int, long, std::hash<long long>, std::equal_to<>>);
    assert(std::is_permutation(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    }

    {
    momo::stdish::unordered_map m({ P{1,1L}, P{2,2L}, P{1,1L}, P{INT_MAX,1L}, P{3,1L} }, 42, std::hash<long long>(), std::equal_to<>(), test_allocator<PC>(0, 44));
    ASSERT_SAME_TYPE(decltype(m), momo::stdish::unordered_map<int, long, std::hash<long long>, std::equal_to<>, test_allocator<PC>>);
    assert(std::is_permutation(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    assert(m.get_allocator().get_id() == 44);
    }

    {
    const P arr[] = { {1,1}, {2,2}, {1,1}, {INT_MAX,1}, {3,1} };
    momo::stdish::unordered_map m(std::begin(arr), std::end(arr), 42, test_allocator<PC>(0, 45));
    ASSERT_SAME_TYPE(decltype(m), momo::stdish::unordered_map<int, long, momo::HashCoder<int>, std::equal_to<int>, test_allocator<PC>>);
    assert(std::is_permutation(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    assert(m.get_allocator().get_id() == 45);
    }

    {
    const P arr[] = { {1,1}, {2,2}, {1,1}, {INT_MAX,1}, {3,1} };
    momo::stdish::unordered_map m(std::begin(arr), std::end(arr), 42, std::hash<long long>(), test_allocator<PC>(0, 46));
    ASSERT_SAME_TYPE(decltype(m), momo::stdish::unordered_map<int, long, std::hash<long long>, std::equal_to<int>, test_allocator<PC>>);
    assert(std::is_permutation(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    assert(m.get_allocator().get_id() == 46);
    }

    {
    momo::stdish::unordered_map m({ P{1,1L}, P{2,2L}, P{1,1L}, P{INT_MAX,1L}, P{3,1L} }, 42, test_allocator<PC>(0, 47));
    ASSERT_SAME_TYPE(decltype(m), momo::stdish::unordered_map<int, long, momo::HashCoder<int>, std::equal_to<int>, test_allocator<PC>>);
    assert(std::is_permutation(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    assert(m.get_allocator().get_id() == 47);
    }

    {
    momo::stdish::unordered_map m({ P{1,1L}, P{2,2L}, P{1,1L}, P{INT_MAX,1L}, P{3,1L} }, 42, std::hash<long long>(), test_allocator<PC>(0, 48));
    ASSERT_SAME_TYPE(decltype(m), momo::stdish::unordered_map<int, long, std::hash<long long>, std::equal_to<int>, test_allocator<PC>>);
    assert(std::is_permutation(m.begin(), m.end(), std::begin(expected_m), std::end(expected_m)));
    assert(m.get_allocator().get_id() == 48);
    }

#if !defined(TEST_GCC)
    {
    // Examples from LWG3025
    momo::stdish::unordered_map m{std::pair{1, 1}, {2, 2}, {3, 3}};
    ASSERT_SAME_TYPE(decltype(m), momo::stdish::unordered_map<int, int>);

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    momo::stdish::unordered_map m2{m.begin(), m.end()};
    ASSERT_SAME_TYPE(decltype(m2), momo::stdish::unordered_map<int, int>);
#endif
    }
#endif

    {
    // Examples from LWG3531
    momo::stdish::unordered_map m1{{std::pair{1, 2}, {3, 4}}, 0};
    ASSERT_SAME_TYPE(decltype(m1), momo::stdish::unordered_map<int, int>);

    using value_type = std::pair<const int, int>;
    momo::stdish::unordered_map m2{{value_type{1, 2}, {3, 4}}, 0};
    ASSERT_SAME_TYPE(decltype(m2), momo::stdish::unordered_map<int, int>);
    }

#if TEST_STD_VER >= 23
    {
      using Range = std::array<P, 0>;
      using Pred = test_equal_to<int>;
      using DefaultPred = std::equal_to<int>;
      using Hash = test_hash<int>;
      using DefaultHash = momo::HashCoder<int>;
      using Alloc = test_allocator<PC>;

      { // (from_range, range)
        momo::stdish::unordered_map c(std::from_range, Range());
        static_assert(std::is_same_v<decltype(c), momo::stdish::unordered_map<int, long>>);
      }

      { // (from_range, range, n)
        momo::stdish::unordered_map c(std::from_range, Range(), std::size_t());
        static_assert(std::is_same_v<decltype(c), momo::stdish::unordered_map<int, long>>);
      }

      { // (from_range, range, n, hash)
        momo::stdish::unordered_map c(std::from_range, Range(), std::size_t(), Hash());
        static_assert(std::is_same_v<decltype(c), momo::stdish::unordered_map<int, long, Hash>>);
      }

      { // (from_range, range, n, hash, pred)
        momo::stdish::unordered_map c(std::from_range, Range(), std::size_t(), Hash(), Pred());
        static_assert(std::is_same_v<decltype(c), momo::stdish::unordered_map<int, long, Hash, Pred>>);
      }

      { // (from_range, range, n, hash, pred, alloc)
        momo::stdish::unordered_map c(std::from_range, Range(), std::size_t(), Hash(), Pred(), Alloc());
        static_assert(std::is_same_v<decltype(c), momo::stdish::unordered_map<int, long, Hash, Pred, Alloc>>);
      }

      { // (from_range, range, n, alloc)
        momo::stdish::unordered_map c(std::from_range, Range(), std::size_t(), Alloc());
        static_assert(std::is_same_v<decltype(c), momo::stdish::unordered_map<int, long, DefaultHash, DefaultPred, Alloc>>);
      }

      // TODO(LWG 2713): uncomment this test once the constructor is added.
      { // (from_range, range, alloc)
        //momo::stdish::unordered_map c(std::from_range, Range(), Alloc());
        //static_assert(std::is_same_v<decltype(c), momo::stdish::unordered_map<int, long, DefaultHash, DefaultPred, Alloc>>);
      }

      { // (from_range, range, n, hash, alloc)
        momo::stdish::unordered_map c(std::from_range, Range(), std::size_t(), Hash(), Alloc());
        static_assert(std::is_same_v<decltype(c), momo::stdish::unordered_map<int, long, Hash, DefaultPred, Alloc>>);
      }
    }
#endif

#if MOMO_VERSION_MAJOR > 3
#if !(defined(TEST_MSVC) && _MSC_VER < 1930) && !(defined(TEST_GCC) && __GNUC__ < 11)
    UnorderedContainerDeductionGuidesSfinaeAway<std::unordered_map, std::unordered_map<int, long>>();
#endif
    UnorderedContainerDeductionGuidesSfinaeAway<momo::stdish::unordered_map, momo::stdish::unordered_map<int, long>>();
#endif

    return 0;
}

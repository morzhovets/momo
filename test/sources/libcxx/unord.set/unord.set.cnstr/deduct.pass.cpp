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

// <unordered_set>

// template<class InputIterator,
//        class Hash = hash<iter-value-type<InputIterator>>,
//        class Pred = equal_to<iter-value-type<InputIterator>>,
//        class Allocator = allocator<iter-value-type<InputIterator>>>
// unordered_set(InputIterator, InputIterator, typename see below::size_type = see below,
//               Hash = Hash(), Pred = Pred(), Allocator = Allocator())
//   -> unordered_set<iter-value-type<InputIterator>,
//                    Hash, Pred, Allocator>;
//
// template<class T, class Hash = hash<T>,
//        class Pred = equal_to<T>, class Allocator = allocator<T>>
// unordered_set(initializer_list<T>, typename see below::size_type = see below,
//               Hash = Hash(), Pred = Pred(), Allocator = Allocator())
//   -> unordered_set<T, Hash, Pred, Allocator>;
//
// template<class InputIterator, class Allocator>
// unordered_set(InputIterator, InputIterator, typename see below::size_type, Allocator)
//   -> unordered_set<iter-value-type<InputIterator>,
//                    hash<iter-value-type<InputIterator>>,
//                    equal_to<iter-value-type<InputIterator>>,
//                    Allocator>;
//
// template<class InputIterator, class Hash, class Allocator>
// unordered_set(InputIterator, InputIterator, typename see below::size_type,
//               Hash, Allocator)
//   -> unordered_set<iter-value-type<InputIterator>, Hash,
//                    equal_to<iter-value-type<InputIterator>>,
//                    Allocator>;
//
// template<class T, class Allocator>
// unordered_set(initializer_list<T>, typename see below::size_type, Allocator)
//   -> unordered_set<T, hash<T>, equal_to<T>, Allocator>;
//
// template<class T, class Hash, class Allocator>
// unordered_set(initializer_list<T>, typename see below::size_type, Hash, Allocator)
//   -> unordered_set<T, Hash, equal_to<T>, Allocator>;
//
// template<ranges::input_range R,
//          class Hash = hash<ranges::range_value_t<R>>,
//          class Pred = equal_to<ranges::range_value_t<R>>,
//          class Allocator = allocator<ranges::range_value_t<R>>>
//   unordered_set(from_range_t, R&&, typename see below::size_type = see below, Hash = Hash(), Pred = Pred(), Allocator = Allocator())
//     -> unordered_set<ranges::range_value_t<R>, Hash, Pred, Allocator>; // C++23
//
// template<ranges::input_range R, class Allocator>
//   unordered_set(from_range_t, R&&, typename see below::size_type, Allocator)
//     -> unordered_set<ranges::range_value_t<R>, hash<ranges::range_value_t<R>>,
//                       equal_to<ranges::range_value_t<R>>, Allocator>; // C++23
//
// template<ranges::input_range R, class Allocator>
//   unordered_set(from_range_t, R&&, Allocator)
//     -> unordered_set<ranges::range_value_t<R>, hash<ranges::range_value_t<R>>,
//                       equal_to<ranges::range_value_t<R>>, Allocator>; // C++23
//
// template<ranges::input_range R, class Hash, class Allocator>
//   unordered_set(from_range_t, R&&, typename see below::size_type, Hash, Allocator)
//     -> unordered_set<ranges::range_value_t<R>, Hash,
//                       equal_to<ranges::range_value_t<R>>, Allocator>; // C++23

int main(int, char**)
{
    const int expected_s[] = {1, 2, 3, INT_MAX};

    {
    const int arr[] = { 1, 2, 1, INT_MAX, 3 };
    momo::stdish::unordered_set s(std::begin(arr), std::end(arr));

    ASSERT_SAME_TYPE(decltype(s), momo::stdish::unordered_set<int>);
    assert(std::is_permutation(s.begin(), s.end(), std::begin(expected_s), std::end(expected_s)));
    }

    {
    const int arr[] = { 1, 2, 1, INT_MAX, 3 };
    momo::stdish::unordered_set s(std::begin(arr), std::end(arr), 42);

    ASSERT_SAME_TYPE(decltype(s), momo::stdish::unordered_set<int>);
    assert(std::is_permutation(s.begin(), s.end(), std::begin(expected_s), std::end(expected_s)));
    }

    {
    const int arr[] = { 1, 2, 1, INT_MAX, 3 };
    momo::stdish::unordered_set s(std::begin(arr), std::end(arr), 42, std::hash<long long>());

    ASSERT_SAME_TYPE(decltype(s), momo::stdish::unordered_set<int, std::hash<long long>>);
    assert(std::is_permutation(s.begin(), s.end(), std::begin(expected_s), std::end(expected_s)));
    }

    {
    const int arr[] = { 1, 2, 1, INT_MAX, 3 };
    momo::stdish::unordered_set s(std::begin(arr), std::end(arr), 42, std::hash<long long>(), test_allocator<int>(0, 40));

    ASSERT_SAME_TYPE(decltype(s), momo::stdish::unordered_set<int, std::hash<long long>, std::equal_to<int>, test_allocator<int>>);
    assert(std::is_permutation(s.begin(), s.end(), std::begin(expected_s), std::end(expected_s)));
    assert(s.get_allocator().get_id() == 40);
    }

    {
    momo::stdish::unordered_set<int, std::hash<long long>, std::equal_to<>, test_allocator<int>> source;
    momo::stdish::unordered_set s(source);
    ASSERT_SAME_TYPE(decltype(s), decltype(source));
    assert(s.size() == 0);
    }

    {
    momo::stdish::unordered_set<int, std::hash<long long>, std::equal_to<>, test_allocator<int>> source;
    momo::stdish::unordered_set s{source};  // braces instead of parens
    ASSERT_SAME_TYPE(decltype(s), decltype(source));
    assert(s.size() == 0);
    }

    {
    momo::stdish::unordered_set<int, std::hash<long long>, std::equal_to<>, test_allocator<int>> source;
    momo::stdish::unordered_set s(source, test_allocator<int>(0, 41));
    ASSERT_SAME_TYPE(decltype(s), decltype(source));
    assert(s.size() == 0);
    assert(s.get_allocator().get_id() == 41);
    }

    {
    momo::stdish::unordered_set<int, std::hash<long long>, std::equal_to<>, test_allocator<int>> source;
    momo::stdish::unordered_set s{source, test_allocator<int>(0, 42)};  // braces instead of parens
    ASSERT_SAME_TYPE(decltype(s), decltype(source));
    assert(s.size() == 0);
    assert(s.get_allocator().get_id() == 42);
    }

    {
    momo::stdish::unordered_set s{ 1, 2, 1, INT_MAX, 3 };

    ASSERT_SAME_TYPE(decltype(s), momo::stdish::unordered_set<int>);
    assert(std::is_permutation(s.begin(), s.end(), std::begin(expected_s), std::end(expected_s)));
    }

    {
    momo::stdish::unordered_set s({ 1, 2, 1, INT_MAX, 3 }, 42);

    ASSERT_SAME_TYPE(decltype(s), momo::stdish::unordered_set<int>);
    assert(std::is_permutation(s.begin(), s.end(), std::begin(expected_s), std::end(expected_s)));
    }

    {
    momo::stdish::unordered_set s({ 1, 2, 1, INT_MAX, 3 }, 42, std::hash<long long>());

    ASSERT_SAME_TYPE(decltype(s), momo::stdish::unordered_set<int, std::hash<long long>>);
    assert(std::is_permutation(s.begin(), s.end(), std::begin(expected_s), std::end(expected_s)));
    }

    {
    momo::stdish::unordered_set s({ 1, 2, 1, INT_MAX, 3 }, 42, std::hash<long long>(), std::equal_to<>());

    ASSERT_SAME_TYPE(decltype(s), momo::stdish::unordered_set<int, std::hash<long long>, std::equal_to<>>);
    assert(std::is_permutation(s.begin(), s.end(), std::begin(expected_s), std::end(expected_s)));
    }

    {
    momo::stdish::unordered_set s({ 1, 2, 1, INT_MAX, 3 }, 42, std::hash<long long>(), std::equal_to<>(), test_allocator<int>(0, 43));

    ASSERT_SAME_TYPE(decltype(s), momo::stdish::unordered_set<int, std::hash<long long>, std::equal_to<>, test_allocator<int>>);
    assert(std::is_permutation(s.begin(), s.end(), std::begin(expected_s), std::end(expected_s)));
    assert(s.get_allocator().get_id() == 43);
    }

    {
    const int arr[] = { 1, 2, 1, INT_MAX, 3 };
    momo::stdish::unordered_set s(std::begin(arr), std::end(arr), 42, test_allocator<int>(0, 44));

    ASSERT_SAME_TYPE(decltype(s), momo::stdish::unordered_set<int, momo::HashCoder<int>, std::equal_to<int>, test_allocator<int>>);
    assert(std::is_permutation(s.begin(), s.end(), std::begin(expected_s), std::end(expected_s)));
    assert(s.get_allocator().get_id() == 44);
    }

    {
    const int arr[] = { 1, 2, 1, INT_MAX, 3 };
    momo::stdish::unordered_set s(std::begin(arr), std::end(arr), 42, std::hash<long long>(), test_allocator<int>(0, 44));

    ASSERT_SAME_TYPE(decltype(s), momo::stdish::unordered_set<int, std::hash<long long>, std::equal_to<int>, test_allocator<int>>);
    assert(std::is_permutation(s.begin(), s.end(), std::begin(expected_s), std::end(expected_s)));
    assert(s.get_allocator().get_id() == 44);
    }

    {
    momo::stdish::unordered_set s({ 1, 2, 1, INT_MAX, 3 }, 42, test_allocator<int>(0, 43));

    ASSERT_SAME_TYPE(decltype(s), momo::stdish::unordered_set<int, momo::HashCoder<int>, std::equal_to<int>, test_allocator<int>>);
    assert(std::is_permutation(s.begin(), s.end(), std::begin(expected_s), std::end(expected_s)));
    assert(s.get_allocator().get_id() == 43);
    }

    {
    momo::stdish::unordered_set s({ 1, 2, 1, INT_MAX, 3 }, 42, std::hash<long long>(), test_allocator<int>(0, 42));

    ASSERT_SAME_TYPE(decltype(s), momo::stdish::unordered_set<int, std::hash<long long>, std::equal_to<int>, test_allocator<int>>);
    assert(std::is_permutation(s.begin(), s.end(), std::begin(expected_s), std::end(expected_s)));
    assert(s.get_allocator().get_id() == 42);
    }

#if TEST_STD_VER >= 23
    {
      using Range = std::array<int, 0>;
      using Pred = test_equal_to<int>;
      using DefaultPred = std::equal_to<int>;
      using Hash = test_hash<int>;
      using DefaultHash = momo::HashCoder<int>;
      using Alloc = test_allocator<int>;

      { // (from_range, range)
        momo::stdish::unordered_set c(std::from_range, Range());
        static_assert(std::is_same_v<decltype(c), momo::stdish::unordered_set<int>>);
      }

      { // (from_range, range, n)
        momo::stdish::unordered_set c(std::from_range, Range(), std::size_t());
        static_assert(std::is_same_v<decltype(c), momo::stdish::unordered_set<int>>);
      }

      { // (from_range, range, n, hash)
        momo::stdish::unordered_set c(std::from_range, Range(), std::size_t(), Hash());
        static_assert(std::is_same_v<decltype(c), momo::stdish::unordered_set<int, Hash>>);
      }

      { // (from_range, range, n, hash, pred)
        momo::stdish::unordered_set c(std::from_range, Range(), std::size_t(), Hash(), Pred());
        static_assert(std::is_same_v<decltype(c), momo::stdish::unordered_set<int, Hash, Pred>>);
      }

      { // (from_range, range, n, hash, pred, alloc)
        momo::stdish::unordered_set c(std::from_range, Range(), std::size_t(), Hash(), Pred(), Alloc());
        static_assert(std::is_same_v<decltype(c), momo::stdish::unordered_set<int, Hash, Pred, Alloc>>);
      }

      { // (from_range, range, n, alloc)
        momo::stdish::unordered_set c(std::from_range, Range(), std::size_t(), Alloc());
        static_assert(std::is_same_v<decltype(c), momo::stdish::unordered_set<int, DefaultHash, DefaultPred, Alloc>>);
      }

      // TODO(LWG 2713): uncomment this test once the constructor is added.
      { // (from_range, range, alloc)
        //momo::stdish::unordered_set c(std::from_range, Range(), Alloc());
        //static_assert(std::is_same_v<decltype(c), momo::stdish::unordered_set<int, DefaultHash, DefaultPred, Alloc>>);
      }

      { // (from_range, range, n, hash, alloc)
        momo::stdish::unordered_set c(std::from_range, Range(), std::size_t(), Hash(), Alloc());
        static_assert(std::is_same_v<decltype(c), momo::stdish::unordered_set<int, Hash, DefaultPred, Alloc>>);
      }
    }
#endif

#if !(defined(TEST_MSVC) && _MSC_VER < 1930) && !(defined(TEST_GCC) && __GNUC__ < 11)
#if MOMO_VERSION_MAJOR > 3
    UnorderedContainerDeductionGuidesSfinaeAway<std::unordered_set, std::unordered_set<int>>();
#endif
#endif

    return 0;
}

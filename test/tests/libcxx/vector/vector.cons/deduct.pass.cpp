//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>
// UNSUPPORTED: c++98, c++03, c++11, c++14
// UNSUPPORTED: libcpp-no-deduction-guides


// template <class InputIterator, class Allocator = allocator<typename iterator_traits<InputIterator>::value_type>>
//    deque(InputIterator, InputIterator, Allocator = Allocator())
//    -> deque<typename iterator_traits<InputIterator>::value_type, Allocator>;
//


//#include <vector>
//#include <iterator>
//#include <cassert>
//#include <cstddef>
//#include <climits> // INT_MAX

//#include "test_macros.h"
//#include "test_iterators.h"
//#include "test_allocator.h"

struct A {};

void main()
{

//  Test the explicit deduction guides
    {
    const int arr[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    momo::stdish::vector vec(std::begin(arr), std::end(arr));

    static_assert(std::is_same<decltype(vec), momo::stdish::vector<int>>::value, "");
    assert(std::equal(vec.begin(), vec.end(), std::begin(arr), std::end(arr)));
    }

    {
    const long arr[] = {INT_MAX, 1L, 2L, 3L };
    momo::stdish::vector vec(std::begin(arr), std::end(arr), std::allocator<long>());
    static_assert(std::is_same<decltype(vec)::value_type, long>::value, "");
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
    static_assert(std::is_same<decltype(vec)::value_type, A>::value, "");
    static_assert(std::is_same<decltype(vec)::allocator_type, std::allocator<A>>::value, "");
    assert(vec.size() == 1);
    }

    {
    momo::stdish::vector vec(1, A{}, test_allocator<A>()); // vector (size_type, T, allocator)
    static_assert(std::is_same<decltype(vec)::value_type, A>::value, "");
    static_assert(std::is_same<decltype(vec)::allocator_type, test_allocator<A>>::value, "");
    assert(vec.size() == 1);
    }

    {
    momo::stdish::vector vec{1U, 2U, 3U, 4U, 5U}; // vector(initializer-list)
    static_assert(std::is_same<decltype(vec)::value_type, unsigned>::value, "");
    assert(vec.size() == 5);
    assert(vec[2] == 3U);
    }

    {
    momo::stdish::vector vec({1.0, 2.0, 3.0, 4.0}, test_allocator<double>()); // vector(initializer-list, allocator)
    static_assert(std::is_same<decltype(vec)::value_type, double>::value, "");
    static_assert(std::is_same<decltype(vec)::allocator_type, test_allocator<double>>::value, "");
    assert(vec.size() == 4);
    assert(vec[3] == 4.0);
    }

    {
    momo::stdish::vector<long double> source;
    momo::stdish::vector vec(source); // vector(vector &)
    static_assert(std::is_same<decltype(vec)::value_type, long double>::value, "");
    static_assert(std::is_same<decltype(vec)::allocator_type, std::allocator<long double>>::value, "");
    assert(vec.size() == 0);
    }


//  A couple of vector<bool> tests, too!
    {
    momo::stdish::vector vec(3, true); // vector(initializer-list)
    static_assert(std::is_same<decltype(vec)::value_type, bool>::value, "");
    static_assert(std::is_same<decltype(vec)::allocator_type, std::allocator<bool>>::value, "");
    assert(vec.size() == 3);
    assert(vec[0] && vec[1] && vec[2]);
    }

    {
    momo::stdish::vector<bool> source;
    momo::stdish::vector vec(source); // vector(vector &)
    static_assert(std::is_same<decltype(vec)::value_type, bool>::value, "");
    static_assert(std::is_same<decltype(vec)::allocator_type, std::allocator<bool>>::value, "");
    assert(vec.size() == 0);
    }
}

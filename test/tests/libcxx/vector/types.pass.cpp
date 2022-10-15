//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <vector>

// Test nested types and default template args:

// template <class T, class Allocator = allocator<T> >
// class vector
// {
// public:
//     typedef T                                        value_type;
//     typedef Allocator                                allocator_type;
//     typedef typename allocator_type::reference       reference;
//     typedef typename allocator_type::const_reference const_reference;
//     typedef implementation-defined                   iterator;
//     typedef implementation-defined                   const_iterator;
//     typedef typename allocator_type::size_type       size_type;
//     typedef typename allocator_type::difference_type difference_type;
//     typedef typename allocator_type::pointer         pointer;
//     typedef typename allocator_type::const_pointer   const_pointer;
//     typedef std::reverse_iterator<iterator>          reverse_iterator;
//     typedef std::reverse_iterator<const_iterator>    const_reverse_iterator;
// };

//#include <vector>
//#include <iterator>
//#include <type_traits>

//#include "test_allocator.h"
//#include "../../Copyable.h"
//#include "min_allocator.h"

//struct A { vector<A> v; }; // incomplete type support

template <class T, class Allocator>
void
test()
{
    typedef vector<T, Allocator> C;

    static_assert((std::is_same<typename C::value_type, T>::value), "");
    static_assert((std::is_same<typename C::value_type, typename Allocator::value_type>::value), "");
    static_assert((std::is_same<typename C::allocator_type, Allocator>::value), "");
    //static_assert((std::is_same<typename C::size_type, typename Allocator::size_type>::value), "");
    //static_assert((std::is_same<typename C::difference_type, typename Allocator::difference_type>::value), "");
    //static_assert((std::is_same<typename C::reference, typename Allocator::reference>::value), "");
    //static_assert((std::is_same<typename C::const_reference, typename Allocator::const_reference>::value), "");
    //static_assert((std::is_same<typename C::pointer, typename Allocator::pointer>::value), "");
    //static_assert((std::is_same<typename C::const_pointer, typename Allocator::const_pointer>::value), "");
    static_assert((std::is_same<typename C::size_type, typename std::allocator_traits<Allocator>::size_type>::value), "");
    static_assert((std::is_same<typename C::difference_type, typename std::allocator_traits<Allocator>::difference_type>::value), "");
    static_assert((std::is_same<typename C::pointer, typename std::allocator_traits<Allocator>::pointer>::value), "");
    static_assert((std::is_same<typename C::const_pointer, typename std::allocator_traits<Allocator>::const_pointer>::value), "");
    static_assert((std::is_same<
        typename std::iterator_traits<typename C::iterator>::iterator_category,
        std::random_access_iterator_tag>::value), "");
    static_assert((std::is_same<
        typename std::iterator_traits<typename C::const_iterator>::iterator_category,
        std::random_access_iterator_tag>::value), "");
    static_assert((std::is_same<
        typename C::reverse_iterator,
        std::reverse_iterator<typename C::iterator> >::value), "");
    static_assert((std::is_same<
        typename C::const_reverse_iterator,
        std::reverse_iterator<typename C::const_iterator> >::value), "");
}

void main()
{
    test<int, test_allocator<int> >();
    test<int*, std::allocator<int*> >();
    test<Copyable, test_allocator<Copyable> >();
    static_assert((std::is_same<vector<char>::allocator_type,
                                std::allocator<char> >::value), "");
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    static_assert((std::is_same<vector<int, min_allocator<int>>::value_type, int>::value), "");
    static_assert((std::is_same<vector<int, min_allocator<int>>::allocator_type, min_allocator<int> >::value), "");
    static_assert((std::is_same<vector<int, min_allocator<int>>::reference, int&>::value), "");
    static_assert((std::is_same<vector<int, min_allocator<int>>::const_reference, const int&>::value), "");
    static_assert((std::is_same<vector<int, min_allocator<int>>::pointer, min_pointer<int>>::value), "");
    static_assert((std::is_same<vector<int, min_allocator<int>>::const_pointer, min_pointer<const int>>::value), "");
#endif
}

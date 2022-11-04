//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <vector>

// template <class Iter>
//   iterator insert(const_iterator position, Iter first, Iter last);

//#include <vector>
//#include <cassert>
//#include <cstddef>

//#include "test_macros.h"
//#include "test_allocator.h"
//#include "test_iterators.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

namespace adl {
struct S {};
void make_move_iterator(S*) {}
}

TEST_CONSTEXPR_CXX20 bool tests()
{
    {
        typedef vector<int> V;
        V v(100);
        int a[] = {1, 2, 3, 4, 5};
        const size_t N = sizeof(a)/sizeof(a[0]);
        V::iterator i = v.insert(v.cbegin() + 10, cpp17_input_iterator<const int*>(a),
                                 cpp17_input_iterator<const int*>(a+N));
        assert(v.size() == 100 + N);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (std::size_t k = 0; k < N; ++j, ++k)
            assert(v[j] == a[k]);
        for (; j < 105; ++j)
            assert(v[j] == 0);
    }
    {
        typedef vector<int> V;
        V v(100);
        int a[] = {1, 2, 3, 4, 5};
        const size_t N = sizeof(a)/sizeof(a[0]);
        V::iterator i = v.insert(v.cbegin() + 10, forward_iterator<const int*>(a),
                                 forward_iterator<const int*>(a+N));
        assert(v.size() == 100 + N);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (std::size_t k = 0; k < N; ++j, ++k)
            assert(v[j] == a[k]);
        for (; j < 105; ++j)
            assert(v[j] == 0);
    }
    {
        typedef vector<int> V;
        V v(100);
        while(v.size() < v.capacity()) v.push_back(0); // force reallocation
        size_t sz = v.size();
        int a[] = {1, 2, 3, 4, 5};
        const size_t N = sizeof(a)/sizeof(a[0]);
        V::iterator i = v.insert(v.cbegin() + 10, forward_iterator<const int*>(a),
                                 forward_iterator<const int*>(a+N));
        assert(v.size() == sz + N);
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (std::size_t k = 0; k < N; ++j, ++k)
            assert(v[j] == a[k]);
        for (; j < v.size(); ++j)
            assert(v[j] == 0);
    }
    {
        typedef vector<int> V;
        V v(100);
        v.reserve(128); // force no reallocation
        size_t sz = v.size();
        int a[] = {1, 2, 3, 4, 5};
        const size_t N = sizeof(a)/sizeof(a[0]);
        V::iterator i = v.insert(v.cbegin() + 10, forward_iterator<const int*>(a),
                                 forward_iterator<const int*>(a+N));
        assert(v.size() == sz + N);
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (std::size_t k = 0; k < N; ++j, ++k)
            assert(v[j] == a[k]);
        for (; j < v.size(); ++j)
            assert(v[j] == 0);
    }
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
    {
        typedef vector<int, limited_allocator<int, 308> > V;
        V v(100);
        int a[] = {1, 2, 3, 4, 5};
        const int N = sizeof(a)/sizeof(a[0]);
        V::iterator i = v.insert(v.cbegin() + 10, cpp17_input_iterator<const int*>(a),
                                 cpp17_input_iterator<const int*>(a+N));
        assert(v.size() == 100 + N);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (std::size_t k = 0; k < N; ++j, ++k)
            assert(v[j] == a[k]);
        for (; j < 105; ++j)
            assert(v[j] == 0);
    }
    {
        typedef vector<int, limited_allocator<int, 300> > V;
        V v(100);
        int a[] = {1, 2, 3, 4, 5};
        const int N = sizeof(a)/sizeof(a[0]);
        V::iterator i = v.insert(v.cbegin() + 10, forward_iterator<const int*>(a),
                                 forward_iterator<const int*>(a+N));
        assert(v.size() == 100 + N);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (std::size_t k = 0; k < N; ++j, ++k)
            assert(v[j] == a[k]);
        for (; j < 105; ++j)
            assert(v[j] == 0);
    }
#endif
#if _LIBCPP_DEBUG >= 1
    {
        vector<int> v(100);
        vector<int> v2(100);
        int a[] = {1, 2, 3, 4, 5};
        const int N = sizeof(a)/sizeof(a[0]);
        LIBCPP_CATCH(v.insert(v2.cbegin() + 10, input_iterator<const int*>(a),
                                        input_iterator<const int*>(a+N)));
        //assert(false);
    }
#endif
//#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR

    {
        typedef vector<int, min_allocator<int> > V;
        V v(100);
        int a[] = {1, 2, 3, 4, 5};
        const int N = sizeof(a)/sizeof(a[0]);
        V::iterator i = v.insert(v.cbegin() + 10, cpp17_input_iterator<const int*>(a),
                                 cpp17_input_iterator<const int*>(a+N));
        assert(v.size() == 100 + N);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (std::size_t k = 0; k < N; ++j, ++k)
            assert(v[j] == a[k]);
        for (; j < 105; ++j)
            assert(v[j] == 0);
    }
    {
        typedef vector<int, min_allocator<int> > V;
        V v(100);
        int a[] = {1, 2, 3, 4, 5};
        const int N = sizeof(a)/sizeof(a[0]);
        V::iterator i = v.insert(v.cbegin() + 10, forward_iterator<const int*>(a),
                                 forward_iterator<const int*>(a+N));
        assert(v.size() == 100 + N);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (std::size_t k = 0; k < N; ++j, ++k)
            assert(v[j] == a[k]);
        for (; j < 105; ++j)
            assert(v[j] == 0);
    }
#if _LIBCPP_DEBUG >= 1
    {
        vector<int, min_allocator<int>> v(100);
        vector<int, min_allocator<int>> v2(100);
        int a[] = {1, 2, 3, 4, 5};
        const int N = sizeof(a)/sizeof(a[0]);
        vector<int, min_allocator<int>>::iterator i = v.insert(v2.cbegin() + 10, input_iterator<const int*>(a),
                                        input_iterator<const int*>(a+N));
        assert(false);
    }
#endif
#endif

    {
        vector<adl::S> s;
        s.insert(s.end(), cpp17_input_iterator<adl::S*>(nullptr), cpp17_input_iterator<adl::S*>(nullptr));
    }

    return true;
}

void main()
{
    tests();
//#if TEST_STD_VER > 17
//    static_assert(tests());
//#endif
}

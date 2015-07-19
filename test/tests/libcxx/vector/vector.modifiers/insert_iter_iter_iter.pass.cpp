//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// template <class Iter>
//   iterator insert(const_iterator position, Iter first, Iter last);

#if _LIBCPP_DEBUG >= 1
#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))
#endif

//#include <vector>
//#include <cassert>
//#include "../../../stack_allocator.h"
//#include "test_iterators.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

void main()
{
    {
        vector<int> v(100);
        int a[] = {1, 2, 3, 4, 5};
        const int N = sizeof(a)/sizeof(a[0]);
        vector<int>::iterator i = v.insert(v.cbegin() + 10, input_iterator<const int*>(a),
                                        input_iterator<const int*>(a+N));
        assert(v.size() == 100 + N);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (int k = 0; k < N; ++j, ++k)
            assert(v[j] == a[k]);
        for (; j < 105; ++j)
            assert(v[j] == 0);
    }
    {
        vector<int> v(100);
        int a[] = {1, 2, 3, 4, 5};
        const int N = sizeof(a)/sizeof(a[0]);
        vector<int>::iterator i = v.insert(v.cbegin() + 10, forward_iterator<const int*>(a),
                                        forward_iterator<const int*>(a+N));
        assert(v.size() == 100 + N);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (int k = 0; k < N; ++j, ++k)
            assert(v[j] == a[k]);
        for (; j < 105; ++j)
            assert(v[j] == 0);
    }
    {
        vector<int> v(100);
        while(v.size() < v.capacity()) v.push_back(0); // force reallocation
        size_t sz = v.size();
        int a[] = {1, 2, 3, 4, 5};
        const unsigned N = sizeof(a)/sizeof(a[0]);
        vector<int>::iterator i = v.insert(v.cbegin() + 10, forward_iterator<const int*>(a),
                                        forward_iterator<const int*>(a+N));
        assert(v.size() == sz + N);
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (unsigned k = 0; k < N; ++j, ++k)
            assert(v[j] == a[k]);
        for (; j < v.size(); ++j)
            assert(v[j] == 0);
    }
    {
        vector<int> v(100);
        v.reserve(128); // force no reallocation
        size_t sz = v.size();
        int a[] = {1, 2, 3, 4, 5};
        const unsigned N = sizeof(a)/sizeof(a[0]);
        vector<int>::iterator i = v.insert(v.cbegin() + 10, forward_iterator<const int*>(a),
                                        forward_iterator<const int*>(a+N));
        assert(v.size() == sz + N);
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (unsigned k = 0; k < N; ++j, ++k)
            assert(v[j] == a[k]);
        for (; j < v.size(); ++j)
            assert(v[j] == 0);
    }
#ifdef LIBCPP_TEST_STACK_ALLOCATOR
    {
        vector<int, stack_allocator<int, 308> > v(100);
        int a[] = {1, 2, 3, 4, 5};
        const int N = sizeof(a)/sizeof(a[0]);
        auto i = v.insert(v.cbegin() + 10, input_iterator<const int*>(a),
                                        input_iterator<const int*>(a+N));
        assert(v.size() == 100 + N);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (int k = 0; k < N; ++j, ++k)
            assert(v[j] == a[k]);
        for (; j < 105; ++j)
            assert(v[j] == 0);
    }
    {
        vector<int, stack_allocator<int, 300> > v(100);
        int a[] = {1, 2, 3, 4, 5};
        const int N = sizeof(a)/sizeof(a[0]);
        auto i = v.insert(v.cbegin() + 10, forward_iterator<const int*>(a),
                                        forward_iterator<const int*>(a+N));
        assert(v.size() == 100 + N);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (int k = 0; k < N; ++j, ++k)
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
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> v(100);
        int a[] = {1, 2, 3, 4, 5};
        const int N = sizeof(a)/sizeof(a[0]);
        vector<int, min_allocator<int>>::iterator i = v.insert(v.cbegin() + 10, input_iterator<const int*>(a),
                                        input_iterator<const int*>(a+N));
        assert(v.size() == 100 + N);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (int k = 0; k < N; ++j, ++k)
            assert(v[j] == a[k]);
        for (; j < 105; ++j)
            assert(v[j] == 0);
    }
    {
        vector<int, min_allocator<int>> v(100);
        int a[] = {1, 2, 3, 4, 5};
        const int N = sizeof(a)/sizeof(a[0]);
        vector<int, min_allocator<int>>::iterator i = v.insert(v.cbegin() + 10, forward_iterator<const int*>(a),
                                        forward_iterator<const int*>(a+N));
        assert(v.size() == 100 + N);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (int k = 0; k < N; ++j, ++k)
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
}

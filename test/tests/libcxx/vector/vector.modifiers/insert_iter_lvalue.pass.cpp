//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <vector>

// iterator insert(const_iterator position, const value_type& x);

//#include <vector>
//#include <cassert>
//#include <cstddef>

//#include "test_macros.h"
//#include "test_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

TEST_CONSTEXPR_CXX20 bool test() {

    {
        vector<int> v(100);
        const int lvalue = 1;
        vector<int>::iterator i = v.insert(v.cbegin() + 10, lvalue);
        assert(v.size() == 101);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        assert(v[j] == 1);
        for (++j; j < 101; ++j)
            assert(v[j] == 0);
    }
    {
        const size_t n = 100;
        vector<int> v(n);
        v.reserve(n + 1);
        const int lvalue = 1;

        // no reallocation expected
        vector<int>::iterator it = v.insert(v.cbegin() + n, lvalue);

        assert(v.size() == n + 1);
        //assert(is_contiguous_container_asan_correct(v));
        assert(it == v.begin() + n);
        for (size_t i = 0; i < n; ++i) {
            assert(v[i] == 0);
        }
        assert(v[n] == lvalue);
    }
    {
        vector<int> v(100);
        while(v.size() < v.capacity()) v.push_back(0); // force reallocation
        size_t sz = v.size();
        const int lvalue = 1;
        vector<int>::iterator i = v.insert(v.cbegin() + 10, lvalue);
        assert(v.size() == sz + 1);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        assert(v[j] == 1);
        for (++j; j < v.size(); ++j)
            assert(v[j] == 0);
    }
    {
        vector<int> v(100);
        while(v.size() < v.capacity()) v.push_back(0);
        v.pop_back(); v.pop_back(); // force no reallocation
        size_t sz = v.size();
        const int lvalue = 1;
        vector<int>::iterator i = v.insert(v.cbegin() + 10, lvalue);
        assert(v.size() == sz + 1);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        assert(v[j] == 1);
        for (++j; j < v.size(); ++j)
            assert(v[j] == 0);
    }
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
    {
        vector<int, limited_allocator<int, 300> > v(100);
        const int lvalue = 1;
        vector<int, limited_allocator<int, 300> >::iterator i = v.insert(v.cbegin() + 10, lvalue);
        assert(v.size() == 101);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        assert(v[j] == 1);
        for (++j; j < 101; ++j)
            assert(v[j] == 0);
    }
#endif
#if _LIBCPP_DEBUG >= 1
    {
        vector<int> v1(3);
        vector<int> v2(3);
        int i = 4;
        LIBCPP_CATCH(v1.insert(v2.begin(), i));
        //assert(false);
    }
#endif
#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> v(100);
        const int lvalue = 1;
        vector<int, min_allocator<int>>::iterator i = v.insert(v.cbegin() + 10, lvalue);
        assert(v.size() == 101);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        assert(v[j] == 1);
        for (++j; j < 101; ++j)
            assert(v[j] == 0);
    }
#if _LIBCPP_DEBUG >= 1
    {
        vector<int, min_allocator<int>> v1(3);
        vector<int, min_allocator<int>> v2(3);
        int i = 4;
        v1.insert(v2.begin(), i);
        assert(false);
    }
#endif
#endif
#endif

    return true;
}

void main()
{
    test();
//#if TEST_STD_VER > 17
//    static_assert(test());
//#endif
}

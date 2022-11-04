//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <vector>

// iterator insert(const_iterator position, size_type n, const value_type& x);

//#include <vector>
//#include <cassert>
//#include <cstddef>

//#include "test_macros.h"
//#include "test_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

TEST_CONSTEXPR_CXX20 bool tests()
{
    {
        vector<int> v(100);
        vector<int>::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        assert(v.size() == 105);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (; j < 15; ++j)
            assert(v[j] == 1);
        for (++j; j < 105; ++j)
            assert(v[j] == 0);
    }
    {
        vector<int> v(100);
        while(v.size() < v.capacity()) v.push_back(0); // force reallocation
        size_t sz = v.size();
        vector<int>::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        assert(v.size() == sz + 5);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (; j < 15; ++j)
            assert(v[j] == 1);
        for (++j; j < v.size(); ++j)
            assert(v[j] == 0);
    }
    {
        vector<int> v(100);
        v.reserve(128); // force no reallocation
        size_t sz = v.size();
        vector<int>::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        assert(v.size() == sz + 5);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (; j < 15; ++j)
            assert(v[j] == 1);
        for (++j; j < v.size(); ++j)
            assert(v[j] == 0);
    }
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
    {
        vector<int, limited_allocator<int, 300> > v(100);
        vector<int, limited_allocator<int, 300> >::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        assert(v.size() == 105);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (; j < 15; ++j)
            assert(v[j] == 1);
        for (++j; j < 105; ++j)
            assert(v[j] == 0);
    }
#endif
#if _LIBCPP_DEBUG >= 1
    {
        vector<int> c1(100);
        vector<int> c2(10);
        LIBCPP_CATCH(c1.insert(c2.cbegin() + 10, 5, 1));
        //assert(false);
    }
#endif
//#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> v(100);
        vector<int, min_allocator<int>>::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        assert(v.size() == 105);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (; j < 15; ++j)
            assert(v[j] == 1);
        for (++j; j < 105; ++j)
            assert(v[j] == 0);
    }
    {
        vector<int, min_allocator<int>> v(100);
        vector<int, min_allocator<int>>::iterator i = v.insert(v.cbegin() + 10, 5, 1);
        assert(v.size() == 105);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == 0);
        for (; j < 15; ++j)
            assert(v[j] == 1);
        for (++j; j < 105; ++j)
            assert(v[j] == 0);
    }
#if _LIBCPP_DEBUG >= 1
    {
        vector<int, min_allocator<int>> c1(100);
        vector<int, min_allocator<int>> c2;
        vector<int, min_allocator<int>>::iterator i = c1.insert(c2.cbegin() + 10, 5, 1);
        assert(false);
    }
#endif
#endif

    return true;
}

void main()
{
    tests();
//#if TEST_STD_VER > 17
//    static_assert(tests());
//#endif
}

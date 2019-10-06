//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// iterator insert(const_iterator position, const value_type& x);

#if _LIBCPP_DEBUG >= 1
//#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))
#endif

//#include <vector>
//#include <cassert>
//#include "../../../stack_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

void main()
{
    {
        vector<int> v(100);
        vector<int>::iterator i = v.insert(v.cbegin() + 10, 1);
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
        vector<int> v(100);
        while(v.size() < v.capacity()) v.push_back(0); // force reallocation
        size_t sz = v.size();
        vector<int>::iterator i = v.insert(v.cbegin() + 10, 1);
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
        vector<int>::iterator i = v.insert(v.cbegin() + 10, 1);
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
#ifdef LIBCPP_TEST_STACK_ALLOCATOR
    {
        vector<int, stack_allocator<int, 300> > v(100);
        vector<int, stack_allocator<int, 300> >::iterator i = v.insert(v.cbegin() + 10, 1);
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
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> v(100);
        vector<int, min_allocator<int>>::iterator i = v.insert(v.cbegin() + 10, 1);
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
}

//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// iterator insert(const_iterator position, value_type&& x);

#if _LIBCPP_DEBUG >= 1
#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))
#endif

//#include <vector>
//#include <cassert>
//#include "../../../stack_allocator.h"
//#include "MoveOnly.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

void main()
{
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    {
        vector<MoveOnly> v(100);
        vector<MoveOnly>::iterator i = v.insert(v.cbegin() + 10, MoveOnly(3));
        assert(v.size() == 101);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == MoveOnly());
        assert(v[j] == MoveOnly(3));
        for (++j; j < 101; ++j)
            assert(v[j] == MoveOnly());
    }
#ifdef LIBCPP_TEST_STACK_ALLOCATOR
    {
        vector<MoveOnly, stack_allocator<MoveOnly, 300> > v(100);
        vector<MoveOnly, stack_allocator<MoveOnly, 300> >::iterator i = v.insert(v.cbegin() + 10, MoveOnly(3));
        assert(v.size() == 101);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == MoveOnly());
        assert(v[j] == MoveOnly(3));
        for (++j; j < 101; ++j)
            assert(v[j] == MoveOnly());
    }
#endif
#if _LIBCPP_DEBUG >= 1
    {
        vector<int> v1(3);
        vector<int> v2(3);
        LIBCPP_CATCH(v1.insert(v2.begin(), 4));
        //assert(false);
    }
#endif
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<MoveOnly, min_allocator<MoveOnly>> v(100);
        vector<MoveOnly, min_allocator<MoveOnly>>::iterator i = v.insert(v.cbegin() + 10, MoveOnly(3));
        assert(v.size() == 101);
        //assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        int j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == MoveOnly());
        assert(v[j] == MoveOnly(3));
        for (++j; j < 101; ++j)
            assert(v[j] == MoveOnly());
    }
#if _LIBCPP_DEBUG >= 1
    {
        vector<int, min_allocator<int>> v1(3);
        vector<int, min_allocator<int>> v2(3);
        v1.insert(v2.begin(), 4);
        assert(false);
    }
#endif
#endif
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
}

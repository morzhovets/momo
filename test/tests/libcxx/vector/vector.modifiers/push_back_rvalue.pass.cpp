//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// void push_back(value_type&& x);

//#include <vector>
//#include <cassert>
//#include "MoveOnly.h"
//#include "../../../stack_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

void main()
{
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    {
        vector<MoveOnly> c;
        c.push_back(MoveOnly(0));
        assert(c.size() == 1);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
        c.push_back(MoveOnly(1));
        assert(c.size() == 2);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
        c.push_back(MoveOnly(2));
        assert(c.size() == 3);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
        c.push_back(MoveOnly(3));
        assert(c.size() == 4);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
        c.push_back(MoveOnly(4));
        assert(c.size() == 5);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly(static_cast<int>(j)));
    }
#ifdef LIBCPP_TEST_STACK_ALLOCATOR
    {
        vector<MoveOnly, stack_allocator<MoveOnly, 15> > c;
        c.push_back(MoveOnly(0));
        assert(c.size() == 1);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly((int)j));
        c.push_back(MoveOnly(1));
        assert(c.size() == 2);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly((int)j));
        c.push_back(MoveOnly(2));
        assert(c.size() == 3);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly((int)j));
        c.push_back(MoveOnly(3));
        assert(c.size() == 4);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly((int)j));
        c.push_back(MoveOnly(4));
        assert(c.size() == 5);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly((int)j));
    }
#endif
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<MoveOnly, min_allocator<MoveOnly>> c;
        c.push_back(MoveOnly(0));
        assert(c.size() == 1);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly((int)j));
        c.push_back(MoveOnly(1));
        assert(c.size() == 2);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly((int)j));
        c.push_back(MoveOnly(2));
        assert(c.size() == 3);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly((int)j));
        c.push_back(MoveOnly(3));
        assert(c.size() == 4);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly((int)j));
        c.push_back(MoveOnly(4));
        assert(c.size() == 5);
        //assert(is_contiguous_container_asan_correct(c));
        for (size_t j = 0; j < c.size(); ++j)
            assert(c[j] == MoveOnly((int)j));
    }
#endif
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
}

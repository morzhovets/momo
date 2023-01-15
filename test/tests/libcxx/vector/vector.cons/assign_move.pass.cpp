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

// UNSUPPORTED: c++03 && !stdlib=libc++

// <vector>

// vector& operator=(vector&& c);

//#include <vector>
//#include <cassert>
//#include "test_macros.h"
//#include "MoveOnly.h"
//#include "test_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

TEST_CONSTEXPR_CXX20 bool tests() {
    {
        vector<MoveOnly, test_allocator<MoveOnly> > l(test_allocator<MoveOnly>(5));
        vector<MoveOnly, test_allocator<MoveOnly> > lo(test_allocator<MoveOnly>(5));
        for (int i = 1; i <= 3; ++i)
        {
            l.push_back(i);
            lo.push_back(i);
        }
        //assert(is_contiguous_container_asan_correct(l));
        //assert(is_contiguous_container_asan_correct(lo));
        vector<MoveOnly, test_allocator<MoveOnly> > l2(test_allocator<MoveOnly>(5));
        l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        //assert(is_contiguous_container_asan_correct(l2));
    }
    {
        vector<MoveOnly, test_allocator<MoveOnly> > l(test_allocator<MoveOnly>(5));
        vector<MoveOnly, test_allocator<MoveOnly> > lo(test_allocator<MoveOnly>(5));
        //assert(is_contiguous_container_asan_correct(l));
        //assert(is_contiguous_container_asan_correct(lo));
        for (int i = 1; i <= 3; ++i)
        {
            l.push_back(i);
            lo.push_back(i);
        }
        //assert(is_contiguous_container_asan_correct(l));
        //assert(is_contiguous_container_asan_correct(lo));
        vector<MoveOnly, test_allocator<MoveOnly> > l2(test_allocator<MoveOnly>(6));
        l2 = std::move(l);
        assert(l2 == lo);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(!l.empty());
#else
        assert(l.empty());
#endif
        assert(l2.get_allocator() == test_allocator<MoveOnly>(6));
        //assert(is_contiguous_container_asan_correct(l2));
    }
    {
        vector<MoveOnly, other_allocator<MoveOnly> > l(other_allocator<MoveOnly>(5));
        vector<MoveOnly, other_allocator<MoveOnly> > lo(other_allocator<MoveOnly>(5));
        //assert(is_contiguous_container_asan_correct(l));
        //assert(is_contiguous_container_asan_correct(lo));
        for (int i = 1; i <= 3; ++i)
        {
            l.push_back(i);
            lo.push_back(i);
        }
        //assert(is_contiguous_container_asan_correct(l));
        //assert(is_contiguous_container_asan_correct(lo));
        vector<MoveOnly, other_allocator<MoveOnly> > l2(other_allocator<MoveOnly>(6));
        l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        //assert(is_contiguous_container_asan_correct(l2));
    }
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<MoveOnly, min_allocator<MoveOnly> > l((min_allocator<MoveOnly>()));
        vector<MoveOnly, min_allocator<MoveOnly> > lo((min_allocator<MoveOnly>()));
        //assert(is_contiguous_container_asan_correct(l));
        //assert(is_contiguous_container_asan_correct(lo));
        for (int i = 1; i <= 3; ++i)
        {
            l.push_back(i);
            lo.push_back(i);
        }
        //assert(is_contiguous_container_asan_correct(l));
        //assert(is_contiguous_container_asan_correct(lo));
        vector<MoveOnly, min_allocator<MoveOnly> > l2((min_allocator<MoveOnly>()));
        l2 = std::move(l);
        assert(l2 == lo);
        assert(l.empty());
        assert(l2.get_allocator() == lo.get_allocator());
        //assert(is_contiguous_container_asan_correct(l2));
    }
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

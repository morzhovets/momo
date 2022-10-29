//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <vector>

// const_pointer data() const;

//#include <vector>
//#include <cassert>

//#include "test_macros.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

struct Nasty {
    TEST_CONSTEXPR Nasty() : i_(0) {}
    TEST_CONSTEXPR Nasty(int i) : i_(i) {}
    TEST_CONSTEXPR_CXX20 ~Nasty() {}

    Nasty * operator&() const { assert(false); return nullptr; }
    int i_;
};

TEST_CONSTEXPR_CXX20 bool tests()
{
#ifndef LIBCXX_TEST_INTCAP_ARRAY
    {
        const vector<int> v;
        assert(v.data() == nullptr);
        //assert(is_contiguous_container_asan_correct(v));
    }
#endif
    {
        const vector<int> v(100);
        assert(v.data() == std::addressof(v.front()));
        //assert(is_contiguous_container_asan_correct(v));
    }
    {
        vector<Nasty> v(100);
        assert(v.data() == std::addressof(v.front()));
        //assert(is_contiguous_container_asan_correct(v));
    }
//#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        const vector<int, min_allocator<int>> v;
        assert(v.data() == nullptr);
        //assert(is_contiguous_container_asan_correct(v));
    }
    {
        const vector<int, min_allocator<int>> v(100);
        assert(v.data() == &v.front());
        //assert(is_contiguous_container_asan_correct(v));
    }
    {
        vector<Nasty, min_allocator<Nasty>> v(100);
        assert(v.data() == std::addressof(v.front()));
        //assert(is_contiguous_container_asan_correct(v));
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

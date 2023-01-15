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

// UNSUPPORTED: c++03

// <vector>

// vector()
//        noexcept(is_nothrow_default_constructible<allocator_type>::value);

// This *was* a conforming extension, but it was adopted in N4258.


//#include <vector>
//#include <cassert>

//#include "test_macros.h"
//#include "MoveOnly.h"
//#include "test_allocator.h"

template <class T>
struct some_alloc
{
    typedef T value_type;
    some_alloc(const some_alloc&);
    void allocate(size_t);
};

TEST_CONSTEXPR_CXX20 bool tests() {
    {
        typedef vector<MoveOnly> C;
        static_assert(std::is_nothrow_default_constructible<C>::value, "");
    }
    {
        typedef vector<MoveOnly, test_allocator<MoveOnly>> C;
        static_assert(std::is_nothrow_default_constructible<C>::value, "");
    }
    {
        typedef vector<MoveOnly, other_allocator<MoveOnly>> C;
        static_assert(!std::is_nothrow_default_constructible<C>::value, "");
    }
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    {
        typedef vector<MoveOnly, some_alloc<MoveOnly>> C;
        static_assert(!std::is_nothrow_default_constructible<C>::value, "");
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

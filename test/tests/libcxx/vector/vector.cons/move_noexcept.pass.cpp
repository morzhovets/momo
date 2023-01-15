//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Modified for https://github.com/morzhovets/momo project.
//
//===----------------------------------------------------------------------===//

// <vector>

// vector(vector&&)
//        noexcept(is_nothrow_move_constructible<allocator_type>::value);

// This tests a conforming extension

//#include <vector>
//#include <cassert>

//#include "MoveOnly.h"
//#include "test_allocator.h"

template <class T>
struct some_alloc
{
    typedef T value_type;
    some_alloc(const some_alloc&);
};

void main()
{
#ifndef _LIBCPP_HAS_NO_NOEXCEPT
//#if __has_feature(cxx_noexcept)
    {
        typedef vector<MoveOnly> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef vector<MoveOnly, test_allocator<MoveOnly>> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef vector<MoveOnly, other_allocator<MoveOnly>> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef vector<MoveOnly, some_alloc<MoveOnly>> C;
        static_assert(!std::is_nothrow_move_constructible<C>::value, "");
    }
#endif
}

//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <set>

// set(set&&)
//        noexcept(is_nothrow_move_constructible<allocator_type>::value &&
//                 is_nothrow_move_constructible<key_compare>::value);

// This tests a conforming extension

//#include <set>
//#include <cassert>

//#include "MoveOnly.h"
//#include "test_allocator.h"

template <class T>
struct some_comp
{
    typedef T value_type;
    some_comp(const some_comp&);
};

void main()
{
#ifndef _LIBCPP_HAS_NO_NOEXCEPT
//#if __has_feature(cxx_noexcept)
    {
        typedef set<MoveOnly> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef set<MoveOnly, std::less<MoveOnly>, test_allocator<MoveOnly>> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef set<MoveOnly, std::less<MoveOnly>, other_allocator<MoveOnly>> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef set<MoveOnly, some_comp<MoveOnly>> C;
        static_assert(!std::is_nothrow_move_constructible<C>::value, "");
    }
#endif
}

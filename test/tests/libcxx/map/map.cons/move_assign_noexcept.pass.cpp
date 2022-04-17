//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// map& operator=(map&& c)
//     noexcept(
//          allocator_type::propagate_on_container_move_assignment::value &&
//          is_nothrow_move_assignable<allocator_type>::value &&
//          is_nothrow_move_assignable<key_compare>::value);

// This tests a conforming extension

//#include <map>
//#include <cassert>

//#include "MoveOnly.h"
//#include "test_allocator.h"

template <class T>
struct some_comp
{
    typedef T value_type;
    some_comp& operator=(const some_comp&);
};

void main()
{
#ifndef _LIBCPP_HAS_NO_NOEXCEPT
//#if __has_feature(cxx_noexcept)
    typedef std::pair<const MoveOnly, MoveOnly> V;
    {
        typedef map<MoveOnly, MoveOnly> C;
        static_assert(std::is_nothrow_move_assignable<C>::value, "");
    }
    {
        typedef map<MoveOnly, MoveOnly, std::less<MoveOnly>, test_allocator<V>> C;
        static_assert(!std::is_nothrow_move_assignable<C>::value, "");
    }
    {
        typedef map<MoveOnly, MoveOnly, std::less<MoveOnly>, other_allocator<V>> C;
        static_assert(std::is_nothrow_move_assignable<C>::value, "");
    }
    {
        typedef map<MoveOnly, MoveOnly, some_comp<MoveOnly>> C;
        static_assert(!std::is_nothrow_move_assignable<C>::value, "");
    }
#endif
}

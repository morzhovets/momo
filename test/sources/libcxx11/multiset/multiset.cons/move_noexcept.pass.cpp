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

// <set>

// multiset(multiset&&)
//        noexcept(is_nothrow_move_constructible<allocator_type>::value &&
//                 is_nothrow_move_constructible<key_compare>::value);

// This tests a conforming extension

// UNSUPPORTED: c++98, c++03

//#include <set>
//#include <cassert>

//#include "test_macros.h"
//#include "MoveOnly.h"
//#include "test_allocator.h"

template <class T>
struct some_comp
{
    typedef T value_type;
    some_comp(const some_comp&);
    bool operator()(const T&, const T&) const { return false; }
};

void main()
{
#if defined(_LIBCPP_VERSION)
    {
        typedef multiset<MoveOnly> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef multiset<MoveOnly, std::less<MoveOnly>, test_allocator<MoveOnly>> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef multiset<MoveOnly, std::less<MoveOnly>, other_allocator<MoveOnly>> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
#endif // _LIBCPP_VERSION
    {
        typedef multiset<MoveOnly, some_comp<MoveOnly>> C;
        static_assert(!std::is_nothrow_move_constructible<C>::value, "");
    }
}

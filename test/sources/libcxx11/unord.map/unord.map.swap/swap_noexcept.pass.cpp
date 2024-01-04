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

// <unordered_map>

// void swap(unordered_map& c)
//     noexcept(!allocator_type::propagate_on_container_swap::value ||
//              __is_nothrow_swappable<allocator_type>::value);

// This tests a conforming extension

//#include <unordered_map>
//#include <cassert>

//#include "MoveOnly.h"
//#include "test_allocator.h"

template <class T>
struct some_comp
{
    typedef T value_type;

    some_comp() {}
    some_comp(const some_comp&) {}
};

template <class T>
struct some_hash
{
    typedef T value_type;
    some_hash() {}
    some_hash(const some_hash&);
};

void main()
{
#ifndef _LIBCPP_HAS_NO_NOEXCEPT
//#if __has_feature(cxx_noexcept)
    {
        typedef unordered_map<MoveOnly, MoveOnly> C;
        C c1, c2;
        static_assert(noexcept(swap(c1, c2)), "");
    }
    {
        typedef unordered_map<MoveOnly, MoveOnly, std::hash<MoveOnly>,
                           std::equal_to<MoveOnly>, test_allocator<std::pair<const MoveOnly, MoveOnly>>> C;
        C c1, c2;
        static_assert(noexcept(swap(c1, c2)), "");
    }
    {
        typedef unordered_map<MoveOnly, MoveOnly, std::hash<MoveOnly>,
                          std::equal_to<MoveOnly>, other_allocator<std::pair<const MoveOnly, MoveOnly>>> C;
        C c1, c2;
        static_assert(noexcept(swap(c1, c2)), "");
    }
    {
        typedef unordered_map<MoveOnly, MoveOnly, some_hash<MoveOnly>> C;
        C c1, c2;
        static_assert(!noexcept(swap(c1, c2)), "");
    }
    {
        typedef unordered_map<MoveOnly, MoveOnly, std::hash<MoveOnly>,
                                                         some_comp<MoveOnly>> C;
        C c1, c2;
        static_assert(!noexcept(swap(c1, c2)), "");
    }
#endif
}

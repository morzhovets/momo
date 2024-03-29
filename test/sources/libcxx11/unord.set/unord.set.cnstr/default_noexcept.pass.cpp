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

// <unordered_set>

// unordered_set()
//    noexcept(
//        is_nothrow_default_constructible<allocator_type>::value &&
//        is_nothrow_default_constructible<key_compare>::value &&
//        is_nothrow_copy_constructible<key_compare>::value);

// This tests a conforming extension

//#include <unordered_set>
//#include <cassert>

//#include "MoveOnly.h"
//#include "test_allocator.h"
//#include "../../../test_hash.h"

template <class T>
struct some_comp
{
    typedef T value_type;
    some_comp();
    some_comp(const some_comp&);
};

template <class T>
struct some_hash
{
    typedef T value_type;
    some_hash();
    some_hash(const some_hash&);
};

void main()
{
#ifndef _LIBCPP_HAS_NO_NOEXCEPT
//#if __has_feature(cxx_noexcept)
    {
        typedef unordered_set<MoveOnly> C;
        static_assert(std::is_nothrow_default_constructible<C>::value, "");
    }
    {
        typedef unordered_set<MoveOnly, std::hash<MoveOnly>,
                           std::equal_to<MoveOnly>, test_allocator<MoveOnly>> C;
        static_assert(std::is_nothrow_default_constructible<C>::value, "");
    }
    {
        typedef unordered_set<MoveOnly, std::hash<MoveOnly>,
                          std::equal_to<MoveOnly>, other_allocator<MoveOnly>> C;
        static_assert(!std::is_nothrow_default_constructible<C>::value, "");
    }
    {
        typedef unordered_set<MoveOnly, some_hash<MoveOnly>> C;
        static_assert(!std::is_nothrow_default_constructible<C>::value, "");
    }
    {
        typedef unordered_set<MoveOnly, std::hash<MoveOnly>,
                                                         some_comp<MoveOnly>> C;
        static_assert(!std::is_nothrow_default_constructible<C>::value, "");
    }
#endif
}

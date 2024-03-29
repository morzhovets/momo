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

// ~unordered_multimap() // implied noexcept;

//#include <unordered_map>
//#include <cassert>

//#include "MoveOnly.h"
//#include "test_allocator.h"

#ifndef _LIBCPP_HAS_NO_NOEXCEPT
//#if __has_feature(cxx_noexcept)

template <class T>
struct some_comp
{
    typedef T value_type;
    ~some_comp() noexcept(false);
};

template <class T>
struct some_hash
{
    typedef T value_type;
    some_hash();
    some_hash(const some_hash&);
    ~some_hash() noexcept(false);
};

#endif

void main()
{
#ifndef _LIBCPP_HAS_NO_NOEXCEPT
//#if __has_feature(cxx_noexcept)
    {
        typedef unordered_multimap<MoveOnly, MoveOnly> C;
        static_assert(std::is_nothrow_destructible<C>::value, "");
    }
    {
        typedef unordered_multimap<MoveOnly, MoveOnly, std::hash<MoveOnly>,
                           std::equal_to<MoveOnly>, test_allocator<std::pair<const MoveOnly, MoveOnly>>> C;
        static_assert(std::is_nothrow_destructible<C>::value, "");
    }
    {
        typedef unordered_multimap<MoveOnly, MoveOnly, std::hash<MoveOnly>,
                          std::equal_to<MoveOnly>, other_allocator<std::pair<const MoveOnly, MoveOnly>>> C;
        static_assert(std::is_nothrow_destructible<C>::value, "");
    }
    {
        typedef unordered_multimap<MoveOnly, MoveOnly, some_hash<MoveOnly>> C;
        static_assert(!std::is_nothrow_destructible<C>::value, "");
    }
    {
        typedef unordered_multimap<MoveOnly, MoveOnly, std::hash<MoveOnly>,
                                                         some_comp<MoveOnly>> C;
        static_assert(!std::is_nothrow_destructible<C>::value, "");
    }
#endif
}

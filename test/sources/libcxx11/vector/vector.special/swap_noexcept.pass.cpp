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

// void swap(vector& c)
//     noexcept(!allocator_type::propagate_on_container_swap::value ||
//              __is_nothrow_swappable<allocator_type>::value);

// This tests a conforming extension

//#include <vector>
//#include <cassert>

//#include "MoveOnly.h"
//#include "test_allocator.h"

template <class T>
struct some_alloc
{
    typedef T value_type;
    
    some_alloc() {}
    some_alloc(const some_alloc&);
    void deallocate(void*, unsigned) {}

    typedef std::true_type propagate_on_container_swap;
};

void main()
{
#ifndef _LIBCPP_HAS_NO_NOEXCEPT
//#if __has_feature(cxx_noexcept)
    {
        typedef vector<MoveOnly> C;
        C c1, c2;
        static_assert(noexcept(swap(c1, c2)), "");
    }
    {
        typedef vector<MoveOnly, test_allocator<MoveOnly>> C;
        C c1, c2;
        static_assert(noexcept(swap(c1, c2)), "");
    }
    {
        typedef vector<MoveOnly, other_allocator<MoveOnly>> C;
        C c1, c2;
        static_assert(noexcept(swap(c1, c2)), "");
    }
    {
        typedef vector<MoveOnly, some_alloc<MoveOnly>> C;
        C c1, c2;
        static_assert(!noexcept(swap(c1, c2)), "");
    }
#endif
}

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

// <map>

// map& operator=(map&& c)
//     noexcept(
//          allocator_type::propagate_on_container_move_assignment::value &&
//          is_nothrow_move_assignable<allocator_type>::value &&
//          is_nothrow_move_assignable<key_compare>::value);

// This tests a conforming extension

// UNSUPPORTED: c++03

template <class T>
struct some_comp
{
    typedef T value_type;
    some_comp& operator=(const some_comp&);
    bool operator()(const T&, const T&) const { return false; }
};

int main(int, char**)
{
    typedef std::pair<const MoveOnly, MoveOnly> V;
    {
        typedef std::map<MoveOnly, MoveOnly> C;
        static_assert(std::is_nothrow_move_assignable<C>::value, "");
    }
    {
        typedef std::map<MoveOnly, MoveOnly, std::less<MoveOnly>, test_allocator<V>> C;
        static_assert(!std::is_nothrow_move_assignable<C>::value, "");
    }
#if defined(LIBCPP_SPECIFIC)
    {
        typedef std::map<MoveOnly, MoveOnly, std::less<MoveOnly>, other_allocator<V>> C;
        static_assert(std::is_nothrow_move_assignable<C>::value, "");
    }
#endif // LIBCPP_SPECIFIC
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    {
        typedef std::map<MoveOnly, MoveOnly, some_comp<MoveOnly>> C;
        static_assert(!std::is_nothrow_move_assignable<C>::value, "");
    }
#endif

  return 0;
}

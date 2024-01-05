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

// map(map&&)
//        noexcept(is_nothrow_move_constructible<allocator_type>::value &&
//                 is_nothrow_move_constructible<key_compare>::value);

// This tests a conforming extension

// UNSUPPORTED: c++03

template <class T>
struct some_comp
{
    typedef T value_type;
    some_comp(const some_comp&);
    bool operator()(const T&, const T&) const { return false; }
};

int main(int, char**)
{
#if defined(LIBCPP_SPECIFIC)
    typedef std::pair<const MoveOnly, MoveOnly> V;
    {
        typedef std::map<MoveOnly, MoveOnly> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef std::map<MoveOnly, MoveOnly, std::less<MoveOnly>, test_allocator<V>> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
    {
        typedef std::map<MoveOnly, MoveOnly, std::less<MoveOnly>, other_allocator<V>> C;
        static_assert(std::is_nothrow_move_constructible<C>::value, "");
    }
#endif // LIBCPP_SPECIFIC
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    {
        typedef std::map<MoveOnly, MoveOnly, some_comp<MoveOnly>> C;
        static_assert(!std::is_nothrow_move_constructible<C>::value, "");
    }
#endif

  return 0;
}

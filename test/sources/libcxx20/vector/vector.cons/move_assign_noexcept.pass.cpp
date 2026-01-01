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

// <vector>

// vector& operator=(vector&& c)
//     noexcept(
//          allocator_type::propagate_on_container_move_assignment::value &&
//          is_nothrow_move_assignable<allocator_type>::value);

// This tests a conforming extension

// UNSUPPORTED: c++03

template <class T>
struct some_alloc
{
    typedef T value_type;
    some_alloc(const some_alloc&);
    T* allocate(size_t);
    void deallocate(void*, size_t) {}
};

template <class T>
struct some_alloc2
{
    typedef T value_type;

    some_alloc2() {}
    some_alloc2(const some_alloc2&);
    T* allocate(size_t);
    void deallocate(void*, size_t) {}

    typedef std::false_type propagate_on_container_move_assignment;
    typedef std::true_type is_always_equal;
};

template <class T>
struct some_alloc3
{
    typedef T value_type;

    some_alloc3() {}
    some_alloc3(const some_alloc3&);
    T* allocate(size_t);
    void deallocate(void*, size_t) {}

    typedef std::false_type propagate_on_container_move_assignment;
    typedef std::false_type is_always_equal;
};


int main(int, char**)
{
    {
        typedef std::vector<MoveOnly> C;
        static_assert(std::is_nothrow_move_assignable<C>::value, "");
    }
    {
        typedef std::vector<MoveOnly, test_allocator<MoveOnly>> C;
        static_assert(!std::is_nothrow_move_assignable<C>::value, "");
    }
    {
        typedef std::vector<MoveOnly, other_allocator<MoveOnly>> C;
        static_assert(std::is_nothrow_move_assignable<C>::value, "");
    }
    {
        typedef std::vector<MoveOnly, some_alloc<MoveOnly>> C;
    //  In C++17, move assignment for allocators are not allowed to throw
#if TEST_STD_VER > 14
        static_assert( std::is_nothrow_move_assignable<C>::value, "");
#else
        static_assert(!std::is_nothrow_move_assignable<C>::value, "");
#endif
    }

#if TEST_STD_VER > 14
    {  // POCMA false, is_always_equal true
        typedef std::vector<MoveOnly, some_alloc2<MoveOnly>> C;
        static_assert( std::is_nothrow_move_assignable<C>::value, "");
    }
#if MOMO_VERSION_MAJOR > 3
    {  // POCMA false, is_always_equal false
        typedef std::vector<MoveOnly, some_alloc3<MoveOnly>> C;
        static_assert(!std::is_nothrow_move_assignable<C>::value, "");
    }
#endif
#endif

    return 0;
}

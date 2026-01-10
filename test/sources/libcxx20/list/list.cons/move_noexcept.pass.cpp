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

// <list>

// list(list&&)
//        noexcept(is_nothrow_move_constructible<allocator_type>::value);

// This tests a conforming extension

// UNSUPPORTED: c++03

template <class T>
struct some_alloc {
  typedef T value_type;
  some_alloc(const some_alloc&);
  T* allocate(std::size_t);
  void deallocate(void*, std::size_t) {}
};

int main(int, char**) {
#if defined(LIBCPP_SPECIFIC)
  {
    typedef std::list<MoveOnly> C;
    static_assert(std::is_nothrow_move_constructible<C>::value, "");
  }
  {
    typedef std::list<MoveOnly, test_allocator<MoveOnly>> C;
    static_assert(std::is_nothrow_move_constructible<C>::value, "");
  }
  {
    typedef std::list<MoveOnly, other_allocator<MoveOnly>> C;
    static_assert(std::is_nothrow_move_constructible<C>::value, "");
  }
#endif // LIBCPP_SPECIFIC
  {
    typedef std::list<MoveOnly, some_alloc<MoveOnly>> C;
    static_assert(!std::is_nothrow_move_constructible<C>::value, "");
  }

  return 0;
}

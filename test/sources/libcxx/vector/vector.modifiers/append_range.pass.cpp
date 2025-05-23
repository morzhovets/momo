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

// UNSUPPORTED: c++03, c++11, c++14, c++17, c++20
// ADDITIONAL_COMPILE_FLAGS(has-fconstexpr-steps): -fconstexpr-steps=2000000

// template<container-compatible-range<T> R>
//   constexpr void append_range(R&& rg); // C++23

// Tested cases:
// - different kinds of insertions (appending an {empty/one-element/mid-sized/long range} into an
//   {empty/one-element/full} container);
// - appending move-only elements;
// - an exception is thrown when copying the elements or when allocating new elements.
/*constexpr*/ bool test() {
  static_assert(test_constraints_append_range<std::vector, int, double>());

  for_all_iterators_and_allocators<int, const int*>([]<class Iter, class Sent, class Alloc>() {
    test_sequence_append_range<std::vector<int, Alloc>, Iter, Sent>([]([[maybe_unused]] auto&& c) {
      LIBCPP_ASSERT(c.__invariants());
      LIBCPP_ASSERT(is_contiguous_container_asan_correct(c));
    });
  });
  test_sequence_append_range_move_only<std::vector>();

  { // Vector may or may not need to reallocate because of the insertion -- make sure to test both cases.
    { // Ensure reallocation happens.
      int in[] = {-1, -2, -3, -4, -5, -6, -7, -8, -9, -10};
      std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8};
      v.shrink_to_fit();
      assert(v.capacity() < v.size() + 3 * std::ranges::size(in));

      v.append_range(in);
      v.append_range(in);
      v.append_range(in);
      assert(std::ranges::equal(v, std::array{1, 2, 3, 4, 5, 6, 7, 8, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10,
        -1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10}));
    }

    { // Ensure no reallocation happens.
      int in[] = {-1, -2, -3, -4, -5, -6, -7, -8, -9, -10};
      std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8};
      v.reserve(v.size() + std::ranges::size(in));
      assert(v.capacity() >= v.size() + std::ranges::size(in));

      v.append_range(in);
      assert(std::ranges::equal(v, std::array{1, 2, 3, 4, 5, 6, 7, 8, -1, -2, -3, -4, -5, -6, -7, -8, -9, -10}));
    }
  }

  return true;
}

int main(int, char**) {
  test();
  //static_assert(test());

  test_append_range_exception_safety_throwing_copy<std::vector>();
  test_append_range_exception_safety_throwing_allocator<std::vector, int>();

  return 0;
}

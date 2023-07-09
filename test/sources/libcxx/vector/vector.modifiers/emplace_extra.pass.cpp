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

// UNSUPPORTED: c++03

// <vector>

// template <class... Args> iterator emplace(const_iterator pos, Args&&... args);

//#include <vector>
//#include <cassert>

//#include "test_macros.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

TEST_CONSTEXPR_CXX20 bool tests() {
    {
        vector<int> v;
        v.reserve(3);
        assert(is_contiguous_container_asan_correct(v));
        v = { 1, 2, 3 };
        v.emplace(v.begin(), v.back());
        assert(v[0] == 3);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        vector<int> v;
        v.reserve(4);
        assert(is_contiguous_container_asan_correct(v));
        v = { 1, 2, 3 };
        v.emplace(v.begin(), v.back());
        assert(v[0] == 3);
        assert(is_contiguous_container_asan_correct(v));
    }
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> v;
        v.reserve(3);
        assert(is_contiguous_container_asan_correct(v));
        v = { 1, 2, 3 };
        v.emplace(v.begin(), v.back());
        assert(v[0] == 3);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        vector<int, min_allocator<int>> v;
        v.reserve(4);
        assert(is_contiguous_container_asan_correct(v));
        v = { 1, 2, 3 };
        v.emplace(v.begin(), v.back());
        assert(v[0] == 3);
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
    {
      vector<int, safe_allocator<int>> v;
      v.reserve(3);
      assert(is_contiguous_container_asan_correct(v));
      v = {1, 2, 3};
      v.emplace(v.begin(), v.back());
      assert(v[0] == 3);
      assert(is_contiguous_container_asan_correct(v));
    }
    {
      vector<int, safe_allocator<int>> v;
      v.reserve(4);
      assert(is_contiguous_container_asan_correct(v));
      v = {1, 2, 3};
      v.emplace(v.begin(), v.back());
      assert(v[0] == 3);
      assert(is_contiguous_container_asan_correct(v));
    }
    {
        vector<int> v;
        v.reserve(8);
        std::size_t old_capacity = v.capacity();
        assert(old_capacity >= 8);

        v.resize(4); // keep the existing capacity
        assert(v.capacity() == old_capacity);

        v.emplace(v.cend(), 42);
        assert(v.size() == 5);
        assert(v.capacity() == old_capacity);
        assert(v[4] == 42);
    }

    return true;
}

void main() {
    tests();
//#if TEST_STD_VER > 17
//    static_assert(tests());
//#endif
}

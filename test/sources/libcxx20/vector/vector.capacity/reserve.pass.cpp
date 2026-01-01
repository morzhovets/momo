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

// void reserve(size_type n);

TEST_CONSTEXPR_CXX20 bool tests() {
    {
    std::vector<int> v;
    v.reserve(10);
    assert(v.capacity() >= 10);
        assert(is_contiguous_container_asan_correct(v));
    }
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
    {
        std::vector<int> v(100);
        assert(v.capacity() == 100);
        v.reserve(50);
        assert(v.size() == 100);
        assert(v.capacity() == 100);
        v.reserve(150);
        assert(v.size() == 100);
#ifdef LIBCXX_TEST_ARRAY
        assert(v.capacity() == 150);
#else
        assert(v.capacity() >= 150);
#endif
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        // Add 1 for implementations that dynamically allocate a container proxy.
        std::vector<int, limited_allocator<int, 250 + 1> > v(100);
        assert(v.capacity() == 100);
        v.reserve(50);
        assert(v.size() == 100);
        assert(v.capacity() == 100);
#ifndef LIBCXX_TEST_INTCAP_ARRAY
        v.reserve(150);
        assert(v.size() == 100);
        assert(v.capacity() == 150);
        assert(is_contiguous_container_asan_correct(v));
#endif
  }
#endif
#if TEST_STD_VER >= 11
  {
    std::vector<int, min_allocator<int>> v;
    v.reserve(10);
    assert(v.capacity() >= 10);
    assert(is_contiguous_container_asan_correct(v));
    }
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
    {
        std::vector<int, min_allocator<int>> v(100);
        assert(v.capacity() == 100);
        v.reserve(50);
        assert(v.size() == 100);
        assert(v.capacity() == 100);
        v.reserve(150);
        assert(v.size() == 100);
#ifdef LIBCXX_TEST_ARRAY
        assert(v.capacity() == 150);
#else
        assert(v.capacity() >= 150);
#endif
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
    {
    std::vector<int, safe_allocator<int>> v;
    v.reserve(10);
    assert(v.capacity() >= 10);
      assert(is_contiguous_container_asan_correct(v));
    }
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
    {
      std::vector<int, safe_allocator<int>> v(100);
      assert(v.capacity() == 100);
      v.reserve(50);
      assert(v.size() == 100);
      assert(v.capacity() == 100);
      v.reserve(150);
      assert(v.size() == 100);
#ifdef LIBCXX_TEST_ARRAY
        assert(v.capacity() == 150);
#else
        assert(v.capacity() >= 150);
#endif
    assert(is_contiguous_container_asan_correct(v));
  }
#endif
#endif

  return true;
}

int main(int, char**) {
  tests();

#if TEST_STD_VER > 17
  //static_assert(tests());
#endif

  return 0;
}

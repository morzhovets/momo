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

// void resize(size_type sz, const value_type& x);

TEST_CONSTEXPR_CXX20 bool tests() {
    {
        std::vector<int> v(100);
        v.resize(50, 1);
        assert(v.size() == 50);
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
        assert(v.capacity() == 100);
#endif
        assert(v == std::vector<int>(50));
        v.resize(200, 1);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        assert(is_contiguous_container_asan_correct(v));
        for (unsigned i = 0; i < 50; ++i)
            assert(v[i] == 0);
        for (unsigned i = 50; i < 200; ++i)
            assert(v[i] == 1);
    }
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
    {
        // Add 1 for implementations that dynamically allocate a container proxy.
        std::vector<int, limited_allocator<int, 300 + 1> > v(100);
        v.resize(50, 1);
        assert(v.size() == 50);
        assert(v.capacity() == 100);
        v.resize(200, 1);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
#if TEST_STD_VER >= 11
    {
        std::vector<int, min_allocator<int>> v(100);
        v.resize(50, 1);
        assert(v.size() == 50);
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
        assert(v.capacity() == 100);
#endif
        assert(is_contiguous_container_asan_correct(v));
        assert((v == std::vector<int, min_allocator<int>>(50)));
        v.resize(200, 1);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        assert(is_contiguous_container_asan_correct(v));
        for (unsigned i = 0; i < 50; ++i)
            assert(v[i] == 0);
        for (unsigned i = 50; i < 200; ++i)
            assert(v[i] == 1);
    }
    {
        std::vector<int, min_allocator<int>> v(100);
        v.resize(50, 1);
        assert(v.size() == 50);
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
        assert(v.capacity() == 100);
#endif
        assert(is_contiguous_container_asan_correct(v));
        v.resize(200, 1);
        assert(v.size() == 200);
        assert(v.capacity() >= 200);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
      std::vector<int, safe_allocator<int>> v(100);
      v.resize(50, 1);
      assert(v.size() == 50);
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
      assert(v.capacity() == 100);
#endif
      assert(is_contiguous_container_asan_correct(v));
      assert((v == std::vector<int, safe_allocator<int>>(50)));
      v.resize(200, 1);
      assert(v.size() == 200);
      assert(v.capacity() >= 200);
      assert(is_contiguous_container_asan_correct(v));
      for (unsigned i = 0; i < 50; ++i)
        assert(v[i] == 0);
      for (unsigned i = 50; i < 200; ++i)
        assert(v[i] == 1);
    }
    {
      std::vector<int, safe_allocator<int>> v(100);
      v.resize(50, 1);
      assert(v.size() == 50);
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
      assert(v.capacity() == 100);
#endif
      assert(is_contiguous_container_asan_correct(v));
      v.resize(200, 1);
      assert(v.size() == 200);
      assert(v.capacity() >= 200);
      assert(is_contiguous_container_asan_correct(v));
    }
#endif

    return true;
}

int main(int, char**)
{
    tests();
#if TEST_STD_VER > 17
    //static_assert(tests());
#endif
    return 0;
}

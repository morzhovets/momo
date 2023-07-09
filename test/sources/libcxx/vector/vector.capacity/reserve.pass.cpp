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

//#include <vector>
//#include <cassert>
//#include <stdexcept>
//#include "test_macros.h"
//#include "test_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

TEST_CONSTEXPR_CXX20 bool tests() {
    {
        vector<int> v;
        v.reserve(10);
        assert(v.capacity() >= 10);
        assert(is_contiguous_container_asan_correct(v));
    }
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
    {
        vector<int> v(100);
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
        vector<int, limited_allocator<int, 250 + 1> > v(100);
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
#ifndef TEST_HAS_NO_EXCEPTIONS
    if (!TEST_IS_CONSTANT_EVALUATED) {
        vector<int> v;
        size_t sz = v.max_size() + 1;

        try {
            v.reserve(sz);
            assert(false);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        } catch (const std::length_error&) {
#else
        } catch (const std::bad_alloc&) {
#endif
            assert(v.size() == 0);
#ifndef LIBCXX_TEST_INTCAP_ARRAY
            assert(v.capacity() == 0);
#endif
        }
    }
    if (!TEST_IS_CONSTANT_EVALUATED) {
        vector<int> v(10, 42);
        int* previous_data = v.data();
        size_t previous_capacity = v.capacity();
        size_t sz = v.max_size() + 1;

        try {
            v.reserve(sz);
            assert(false);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        } catch (const std::length_error&) {
#else
        } catch (const std::bad_alloc&) {
#endif
            assert(v.size() == 10);
            assert(v.capacity() == previous_capacity);
            assert(v.data() == previous_data);

            for (size_t i = 0; i < 10; ++i) {
                assert(v[i] == 42);
            }
        }
    }
#endif
#endif
#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> v;
        v.reserve(10);
        assert(v.capacity() >= 10);
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        vector<int, min_allocator<int>> v(100);
        assert(v.capacity() == 100);
        v.reserve(50);
        assert(v.size() == 100);
        assert(v.capacity() == 100);
        v.reserve(150);
        assert(v.size() == 100);
        assert(v.capacity() == 150);
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
    {
      vector<int, safe_allocator<int>> v;
      v.reserve(10);
      assert(v.capacity() >= 10);
      assert(is_contiguous_container_asan_correct(v));
    }
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
    {
      vector<int, safe_allocator<int>> v(100);
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
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
#ifndef TEST_HAS_NO_EXCEPTIONS
    if (!TEST_IS_CONSTANT_EVALUATED) {
        vector<int, limited_allocator<int, 100> > v;
        v.reserve(50);
        assert(v.capacity() == 50);
        assert(is_contiguous_container_asan_correct(v));
        try {
            v.reserve(101);
            assert(false);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        } catch (const std::length_error&) {
#else
        } catch (const std::bad_alloc&) {
#endif
            // no-op
        }
        assert(v.capacity() == 50);
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
#endif

    return true;
}

void main()
{
  tests();
//#if TEST_STD_VER > 17
//  static_assert(tests());
//#endif
}

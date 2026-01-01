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

// size_type capacity() const;

TEST_CONSTEXPR_CXX20 bool tests()
{
#ifndef LIBCXX_TEST_INTCAP_ARRAY
    {
        std::vector<int> v;
        assert(v.capacity() == 0);
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
    {
        std::vector<int> v(100);
        assert(v.capacity() == 100);
        v.push_back(0);
        assert(v.capacity() > 101);
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
#if TEST_STD_VER >= 11
#ifndef LIBCXX_TEST_INTCAP_ARRAY
    {
        std::vector<int, min_allocator<int>> v;
        assert(v.capacity() == 0);
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
    {
        std::vector<int, min_allocator<int>> v(100);
        assert(v.capacity() == 100);
        v.push_back(0);
        assert(v.capacity() > 101);
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
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

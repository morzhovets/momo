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

// UNSUPPORTED: c++03 && !stdlib=libc++

// <vector>

// iterator insert(const_iterator position, value_type&& x);

TEST_CONSTEXPR_CXX20 bool tests()
{
    {
        std::vector<MoveOnly> v(100);
        std::vector<MoveOnly>::iterator i = v.insert(v.cbegin() + 10, MoveOnly(3));
        assert(v.size() == 101);
        assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == MoveOnly());
        assert(v[j] == MoveOnly(3));
        for (++j; j < 101; ++j)
            assert(v[j] == MoveOnly());
    }
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
    {
        std::vector<MoveOnly, limited_allocator<MoveOnly, 300> > v(100);
        std::vector<MoveOnly, limited_allocator<MoveOnly, 300> >::iterator i = v.insert(v.cbegin() + 10, MoveOnly(3));
        assert(v.size() == 101);
        assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == MoveOnly());
        assert(v[j] == MoveOnly(3));
        for (++j; j < 101; ++j)
            assert(v[j] == MoveOnly());
    }
#endif
    {
        std::vector<MoveOnly, min_allocator<MoveOnly> > v(100);
        std::vector<MoveOnly, min_allocator<MoveOnly> >::iterator i = v.insert(v.cbegin() + 10, MoveOnly(3));
        assert(v.size() == 101);
        assert(is_contiguous_container_asan_correct(v));
        assert(i == v.begin() + 10);
        size_t j;
        for (j = 0; j < 10; ++j)
            assert(v[j] == MoveOnly());
        assert(v[j] == MoveOnly(3));
        for (++j; j < 101; ++j)
            assert(v[j] == MoveOnly());
    }
    {
      std::vector<MoveOnly, safe_allocator<MoveOnly> > v(100);
      std::vector<MoveOnly, safe_allocator<MoveOnly> >::iterator i = v.insert(v.cbegin() + 10, MoveOnly(3));
      assert(v.size() == 101);
      assert(is_contiguous_container_asan_correct(v));
      assert(i == v.begin() + 10);
      size_t j;
      for (j = 0; j < 10; ++j)
        assert(v[j] == MoveOnly());
      assert(v[j] == MoveOnly(3));
      for (++j; j < 101; ++j)
        assert(v[j] == MoveOnly());
    }

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

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

// const_pointer data() const;

struct Nasty {
    TEST_CONSTEXPR Nasty() : i_(0) {}
    TEST_CONSTEXPR Nasty(int i) : i_(i) {}
    TEST_CONSTEXPR_CXX20 ~Nasty() {}

    Nasty * operator&() const { assert(false); return nullptr; }
    int i_;
};

TEST_CONSTEXPR_CXX20 bool tests()
{
#ifndef LIBCXX_TEST_INTCAP_ARRAY
    {
        const std::vector<int> v;
        assert(v.data() == nullptr);
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
    {
        const std::vector<int> v(100);
        assert(v.data() == std::addressof(v.front()));
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        const std::vector<Nasty> v(100);
        assert(v.data() == std::addressof(v.front()));
        assert(is_contiguous_container_asan_correct(v));
    }
#if TEST_STD_VER >= 11
#ifndef LIBCXX_TEST_INTCAP_ARRAY
    {
        const std::vector<int, min_allocator<int>> v;
        assert(v.data() == nullptr);
        assert(is_contiguous_container_asan_correct(v));
    }
#endif
    {
        const std::vector<int, min_allocator<int>> v(100);
        assert(v.data() == &v.front());
        assert(is_contiguous_container_asan_correct(v));
    }
    {
        const std::vector<Nasty, min_allocator<Nasty>> v(100);
        assert(v.data() == std::addressof(v.front()));
        assert(is_contiguous_container_asan_correct(v));
    }
#ifndef LIBCXX_TEST_INTCAP_ARRAY
    {
      const std::vector<int, safe_allocator<int>> v;
      assert(v.data() == nullptr);
      assert(is_contiguous_container_asan_correct(v));
    }
#endif
    {
      const std::vector<int, safe_allocator<int>> v(100);
      assert(v.data() == &v.front());
      assert(is_contiguous_container_asan_correct(v));
    }
    {
      const std::vector<Nasty, safe_allocator<Nasty>> v(100);
      assert(v.data() == std::addressof(v.front()));
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

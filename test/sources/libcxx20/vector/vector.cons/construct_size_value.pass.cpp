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

// vector(size_type n, const value_type& x);

template <class C>
TEST_CONSTEXPR_CXX20 void
test(typename C::size_type n, const typename C::value_type& x)
{
    C c(n, x);
    LIBCPP_ASSERT(c.__invariants());
    assert(c.size() == n);
    LIBCPP_ASSERT(is_contiguous_container_asan_correct(c));
    for (typename C::const_iterator i = c.cbegin(), e = c.cend(); i != e; ++i)
        assert(*i == x);
}

TEST_CONSTEXPR_CXX20 bool tests() {
    test<std::vector<int> >(0, 3);
    test<std::vector<int> >(50, 3);
#ifndef LIBCXX_TEST_SEGMENTED_ARRAY
    // Add 1 for implementations that dynamically allocate a container proxy.
    test<std::vector<int, limited_allocator<int, 50 + 1> > >(0, 5);
    test<std::vector<int, limited_allocator<int, 50 + 1> > >(50, 5);
    test<std::vector<int, limited_allocator<int, 50 + 1> > >(50, 5);
#endif
#if TEST_STD_VER >= 11
    test<std::vector<int, min_allocator<int>> >(0, 3);
    test<std::vector<int, min_allocator<int>> >(50, 3);
    test<std::vector<int, safe_allocator<int>> >(0, 3);
    test<std::vector<int, safe_allocator<int>> >(50, 3);
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

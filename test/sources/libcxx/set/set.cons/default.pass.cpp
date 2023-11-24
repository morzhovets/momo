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

// <set>

// class set

// set();

void main()
{
    {
    std::set<int> m;
    assert(m.empty());
    assert(m.begin() == m.end());
    }
#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    std::set<int, std::less<int>, min_allocator<int>> m;
    assert(m.empty());
    assert(m.begin() == m.end());
    }
#endif

    {
    typedef explicit_allocator<int> A;
        {
        std::set<int, std::less<int>, A> m;
        assert(m.empty());
        assert(m.begin() == m.end());
        }
        {
        A a;
        std::set<int, std::less<int>, A> m(a);
        assert(m.empty());
        assert(m.begin() == m.end());
        }
    }
    {
    std::set<int> m = {};
    assert(m.empty());
    assert(m.begin() == m.end());
    }
#endif
}

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

// size_type erase(const key_type& k);

void main()
{
    {
        typedef set<int> M;
        typedef int V;
        typedef M::size_type I;
        V ar[] =
        {
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8
        };
        M m(ar, ar + sizeof(ar)/sizeof(ar[0]));
        assert(m.size() == 8);
        I i = m.erase(9);
        assert(m.size() == 8);
        assert(i == 0);
        assert(*std::next(m.begin(), 0) == 1);
        assert(*std::next(m.begin(), 1) == 2);
        assert(*std::next(m.begin(), 2) == 3);
        assert(*std::next(m.begin(), 3) == 4);
        assert(*std::next(m.begin(), 4) == 5);
        assert(*std::next(m.begin(), 5) == 6);
        assert(*std::next(m.begin(), 6) == 7);
        assert(*std::next(m.begin(), 7) == 8);

        i = m.erase(4);
        assert(m.size() == 7);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 1);
        assert(*std::next(m.begin(), 1) == 2);
        assert(*std::next(m.begin(), 2) == 3);
        assert(*std::next(m.begin(), 3) == 5);
        assert(*std::next(m.begin(), 4) == 6);
        assert(*std::next(m.begin(), 5) == 7);
        assert(*std::next(m.begin(), 6) == 8);

        i = m.erase(1);
        assert(m.size() == 6);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 3);
        assert(*std::next(m.begin(), 2) == 5);
        assert(*std::next(m.begin(), 3) == 6);
        assert(*std::next(m.begin(), 4) == 7);
        assert(*std::next(m.begin(), 5) == 8);

        i = m.erase(8);
        assert(m.size() == 5);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 3);
        assert(*std::next(m.begin(), 2) == 5);
        assert(*std::next(m.begin(), 3) == 6);
        assert(*std::next(m.begin(), 4) == 7);

        i = m.erase(3);
        assert(m.size() == 4);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 5);
        assert(*std::next(m.begin(), 2) == 6);
        assert(*std::next(m.begin(), 3) == 7);

        i = m.erase(6);
        assert(m.size() == 3);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 5);
        assert(*std::next(m.begin(), 2) == 7);

        i = m.erase(7);
        assert(m.size() == 2);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 5);

        i = m.erase(2);
        assert(m.size() == 1);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 5);

        i = m.erase(5);
        assert(m.size() == 0);
        assert(i == 1);
    }
#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef set<int, std::less<int>, min_allocator<int>> M;
        typedef int V;
        typedef M::size_type I;
        V ar[] =
        {
            1,
            2,
            3,
            4,
            5,
            6,
            7,
            8
        };
        M m(ar, ar + sizeof(ar)/sizeof(ar[0]));
        assert(m.size() == 8);
        I i = m.erase(9);
        assert(m.size() == 8);
        assert(i == 0);
        assert(*std::next(m.begin(), 0) == 1);
        assert(*std::next(m.begin(), 1) == 2);
        assert(*std::next(m.begin(), 2) == 3);
        assert(*std::next(m.begin(), 3) == 4);
        assert(*std::next(m.begin(), 4) == 5);
        assert(*std::next(m.begin(), 5) == 6);
        assert(*std::next(m.begin(), 6) == 7);
        assert(*std::next(m.begin(), 7) == 8);

        i = m.erase(4);
        assert(m.size() == 7);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 1);
        assert(*std::next(m.begin(), 1) == 2);
        assert(*std::next(m.begin(), 2) == 3);
        assert(*std::next(m.begin(), 3) == 5);
        assert(*std::next(m.begin(), 4) == 6);
        assert(*std::next(m.begin(), 5) == 7);
        assert(*std::next(m.begin(), 6) == 8);

        i = m.erase(1);
        assert(m.size() == 6);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 3);
        assert(*std::next(m.begin(), 2) == 5);
        assert(*std::next(m.begin(), 3) == 6);
        assert(*std::next(m.begin(), 4) == 7);
        assert(*std::next(m.begin(), 5) == 8);

        i = m.erase(8);
        assert(m.size() == 5);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 3);
        assert(*std::next(m.begin(), 2) == 5);
        assert(*std::next(m.begin(), 3) == 6);
        assert(*std::next(m.begin(), 4) == 7);

        i = m.erase(3);
        assert(m.size() == 4);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 5);
        assert(*std::next(m.begin(), 2) == 6);
        assert(*std::next(m.begin(), 3) == 7);

        i = m.erase(6);
        assert(m.size() == 3);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 5);
        assert(*std::next(m.begin(), 2) == 7);

        i = m.erase(7);
        assert(m.size() == 2);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 5);

        i = m.erase(2);
        assert(m.size() == 1);
        assert(i == 1);
        assert(*std::next(m.begin(), 0) == 5);

        i = m.erase(5);
        assert(m.size() == 0);
        assert(i == 1);
    }
#endif
#endif
}

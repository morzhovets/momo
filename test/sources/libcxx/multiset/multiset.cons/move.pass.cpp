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

// <set>

// class multiset

// multiset(multiset&& s);

int main(int, char**)
{
    {
        typedef int V;
        typedef test_less<int> C;
        typedef test_allocator<V> A;
        std::multiset<int, C, A> mo(C(5), A(7));
        std::multiset<int, C, A> m = std::move(mo);
        assert(m.get_allocator() == A(7));
        assert(m.key_comp() == C(5));
        assert(m.size() == 0);
        assert(std::distance(m.begin(), m.end()) == 0);

        assert(mo.get_allocator() == A(7));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(mo.get_allocator().get_id() == test_alloc_base::moved_value);
#endif
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(std::distance(mo.begin(), mo.end()) == 0);
    }
    {
        typedef int V;
        V ar[] =
        {
            1,
            1,
            1,
            2,
            2,
            2,
            3,
            3,
            3
        };
        typedef test_less<int> C;
        typedef test_allocator<V> A;
        std::multiset<int, C, A> mo(ar, ar+sizeof(ar)/sizeof(ar[0]), C(5), A(7));
        std::multiset<int, C, A> m = std::move(mo);
        assert(m.get_allocator() == A(7));
        assert(m.key_comp() == C(5));
        assert(m.size() == 9);
        assert(std::distance(m.begin(), m.end()) == 9);
        assert(*std::next(m.begin(), 0) == 1);
        assert(*std::next(m.begin(), 1) == 1);
        assert(*std::next(m.begin(), 2) == 1);
        assert(*std::next(m.begin(), 3) == 2);
        assert(*std::next(m.begin(), 4) == 2);
        assert(*std::next(m.begin(), 5) == 2);
        assert(*std::next(m.begin(), 6) == 3);
        assert(*std::next(m.begin(), 7) == 3);
        assert(*std::next(m.begin(), 8) == 3);

        assert(mo.get_allocator() == A(7));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(mo.get_allocator().get_id() == test_alloc_base::moved_value);
#endif
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(std::distance(mo.begin(), mo.end()) == 0);
    }
    {
        typedef int V;
        V ar[] =
        {
            1,
            1,
            1,
            2,
            2,
            2,
            3,
            3,
            3
        };
        typedef test_less<int> C;
        typedef min_allocator<V> A;
        std::multiset<int, C, A> mo(ar, ar+sizeof(ar)/sizeof(ar[0]), C(5), A());
        std::multiset<int, C, A> m = std::move(mo);
        assert(m.get_allocator() == A());
        assert(m.key_comp() == C(5));
        assert(m.size() == 9);
        assert(std::distance(m.begin(), m.end()) == 9);
        assert(*std::next(m.begin(), 0) == 1);
        assert(*std::next(m.begin(), 1) == 1);
        assert(*std::next(m.begin(), 2) == 1);
        assert(*std::next(m.begin(), 3) == 2);
        assert(*std::next(m.begin(), 4) == 2);
        assert(*std::next(m.begin(), 5) == 2);
        assert(*std::next(m.begin(), 6) == 3);
        assert(*std::next(m.begin(), 7) == 3);
        assert(*std::next(m.begin(), 8) == 3);

        assert(mo.get_allocator() == A());
        assert(mo.key_comp() == C(5));
        assert(mo.size() == 0);
        assert(std::distance(mo.begin(), mo.end()) == 0);
    }

  return 0;
}

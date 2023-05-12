//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Modified for https://github.com/morzhovets/momo project.
//
//===----------------------------------------------------------------------===//

// <set>

// class set

// iterator erase(const_iterator position);

//#include <set>
//#include <cassert>

//#include "min_allocator.h"

struct TemplateConstructor
{
    template<typename T>
    TemplateConstructor (const T&) {}
};

bool operator<(const TemplateConstructor&, const TemplateConstructor&) { return false; }

void main()
{
    {
        typedef set<int> M;
        typedef int V;
        typedef M::iterator I;
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
        I i = m.erase(next(m.cbegin(), 3));
        assert(m.size() == 7);
        assert(i == next(m.begin(), 3));
        assert(*std::next(m.begin(), 0) == 1);
        assert(*std::next(m.begin(), 1) == 2);
        assert(*std::next(m.begin(), 2) == 3);
        assert(*std::next(m.begin(), 3) == 5);
        assert(*std::next(m.begin(), 4) == 6);
        assert(*std::next(m.begin(), 5) == 7);
        assert(*std::next(m.begin(), 6) == 8);

        i = m.erase(next(m.cbegin(), 0));
        assert(m.size() == 6);
        assert(i == m.begin());
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 3);
        assert(*std::next(m.begin(), 2) == 5);
        assert(*std::next(m.begin(), 3) == 6);
        assert(*std::next(m.begin(), 4) == 7);
        assert(*std::next(m.begin(), 5) == 8);

        i = m.erase(next(m.cbegin(), 5));
        assert(m.size() == 5);
        assert(i == m.end());
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 3);
        assert(*std::next(m.begin(), 2) == 5);
        assert(*std::next(m.begin(), 3) == 6);
        assert(*std::next(m.begin(), 4) == 7);

        i = m.erase(next(m.cbegin(), 1));
        assert(m.size() == 4);
        assert(i == next(m.begin()));
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 5);
        assert(*std::next(m.begin(), 2) == 6);
        assert(*std::next(m.begin(), 3) == 7);

        i = m.erase(next(m.cbegin(), 2));
        assert(m.size() == 3);
        assert(i == next(m.begin(), 2));
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 5);
        assert(*std::next(m.begin(), 2) == 7);

        i = m.erase(next(m.cbegin(), 2));
        assert(m.size() == 2);
        assert(i == next(m.begin(), 2));
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 5);

        i = m.erase(next(m.cbegin(), 0));
        assert(m.size() == 1);
        assert(i == next(m.begin(), 0));
        assert(*std::next(m.begin(), 0) == 5);

        i = m.erase(m.cbegin());
        assert(m.size() == 0);
        assert(i == m.begin());
        assert(i == m.end());
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef set<int, std::less<int>, min_allocator<int>> M;
        typedef int V;
        typedef M::iterator I;
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
        I i = m.erase(next(m.cbegin(), 3));
        assert(m.size() == 7);
        assert(i == next(m.begin(), 3));
        assert(*std::next(m.begin(), 0) == 1);
        assert(*std::next(m.begin(), 1) == 2);
        assert(*std::next(m.begin(), 2) == 3);
        assert(*std::next(m.begin(), 3) == 5);
        assert(*std::next(m.begin(), 4) == 6);
        assert(*std::next(m.begin(), 5) == 7);
        assert(*std::next(m.begin(), 6) == 8);

        i = m.erase(next(m.cbegin(), 0));
        assert(m.size() == 6);
        assert(i == m.begin());
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 3);
        assert(*std::next(m.begin(), 2) == 5);
        assert(*std::next(m.begin(), 3) == 6);
        assert(*std::next(m.begin(), 4) == 7);
        assert(*std::next(m.begin(), 5) == 8);

        i = m.erase(next(m.cbegin(), 5));
        assert(m.size() == 5);
        assert(i == m.end());
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 3);
        assert(*std::next(m.begin(), 2) == 5);
        assert(*std::next(m.begin(), 3) == 6);
        assert(*std::next(m.begin(), 4) == 7);

        i = m.erase(next(m.cbegin(), 1));
        assert(m.size() == 4);
        assert(i == next(m.begin()));
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 5);
        assert(*std::next(m.begin(), 2) == 6);
        assert(*std::next(m.begin(), 3) == 7);

        i = m.erase(next(m.cbegin(), 2));
        assert(m.size() == 3);
        assert(i == next(m.begin(), 2));
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 5);
        assert(*std::next(m.begin(), 2) == 7);

        i = m.erase(next(m.cbegin(), 2));
        assert(m.size() == 2);
        assert(i == next(m.begin(), 2));
        assert(*std::next(m.begin(), 0) == 2);
        assert(*std::next(m.begin(), 1) == 5);

        i = m.erase(next(m.cbegin(), 0));
        assert(m.size() == 1);
        assert(i == next(m.begin(), 0));
        assert(*std::next(m.begin(), 0) == 5);

        i = m.erase(m.cbegin());
        assert(m.size() == 0);
        assert(i == m.begin());
        assert(i == m.end());
    }
#endif
//#if __cplusplus >= 201402L
    {
    //  This is LWG #2059
        typedef TemplateConstructor T;
        typedef set<T> C;
        typedef C::iterator I;

        C c;
        T a{0};
        I it = c.find(a);
        if (it != c.end())
            c.erase(it);
    }
//#endif
}

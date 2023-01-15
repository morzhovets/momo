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

//       iterator begin();
// const_iterator begin() const;
//       iterator end();
// const_iterator end()   const;
//
//       reverse_iterator rbegin();
// const_reverse_iterator rbegin() const;
//       reverse_iterator rend();
// const_reverse_iterator rend()   const;
//
// const_iterator         cbegin()  const;
// const_iterator         cend()    const;
// const_reverse_iterator crbegin() const;
// const_reverse_iterator crend()   const;

//#include <set>
//#include <cassert>

//#include "min_allocator.h"

void main()
{
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
            3,
            4,
            4,
            4,
            5,
            5,
            5,
            6,
            6,
            6,
            7,
            7,
            7,
            8,
            8,
            8
        };
        set<int> m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        assert(momo::internal::UIntMath<>::Dist(m.begin(), m.end()) == m.size());
        assert(momo::internal::UIntMath<>::Dist(m.rbegin(), m.rend()) == m.size());
        set<int>::iterator i;
        i = m.begin();
        set<int>::const_iterator k = i;
        assert(i == k);
        for (size_t j = 1; j <= m.size(); ++j, ++i)
            assert(*i == static_cast<int>(j));
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
            3,
            4,
            4,
            4,
            5,
            5,
            5,
            6,
            6,
            6,
            7,
            7,
            7,
            8,
            8,
            8
        };
        const set<int> m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        assert(momo::internal::UIntMath<>::Dist(m.begin(), m.end()) == m.size());
        assert(momo::internal::UIntMath<>::Dist(m.cbegin(), m.cend()) == m.size());
        assert(momo::internal::UIntMath<>::Dist(m.rbegin(), m.rend()) == m.size());
        assert(momo::internal::UIntMath<>::Dist(m.crbegin(), m.crend()) == m.size());
        set<int>::const_iterator i;
        i = m.begin();
        for (size_t j = 1; j <= m.size(); ++j, ++i)
            assert(*i == static_cast<int>(j));
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
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
            3,
            4,
            4,
            4,
            5,
            5,
            5,
            6,
            6,
            6,
            7,
            7,
            7,
            8,
            8,
            8
        };
        set<int, std::less<int>, min_allocator<int>> m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        assert(std::distance(m.begin(), m.end()) == m.size());
        assert(std::distance(m.rbegin(), m.rend()) == m.size());
        set<int, std::less<int>, min_allocator<int>>::iterator i;
        i = m.begin();
        set<int, std::less<int>, min_allocator<int>>::const_iterator k = i;
        assert(i == k);
        for (int j = 1; j <= m.size(); ++j, ++i)
            assert(*i == j);
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
            3,
            4,
            4,
            4,
            5,
            5,
            5,
            6,
            6,
            6,
            7,
            7,
            7,
            8,
            8,
            8
        };
        const set<int, std::less<int>, min_allocator<int>> m(ar, ar+sizeof(ar)/sizeof(ar[0]));
        assert(std::distance(m.begin(), m.end()) == m.size());
        assert(std::distance(m.cbegin(), m.cend()) == m.size());
        assert(std::distance(m.rbegin(), m.rend()) == m.size());
        assert(std::distance(m.crbegin(), m.crend()) == m.size());
        set<int, std::less<int>, min_allocator<int>>::const_iterator i;
        i = m.begin();
        for (int j = 1; j <= m.size(); ++j, ++i)
            assert(*i == j);
    }
#endif
//#if _LIBCPP_STD_VER > 11
    { // N3644 testing
        typedef set<int> C;
        C::iterator ii1{}, ii2{};
        C::iterator ii4 = ii1;
        C::const_iterator cii{};
        assert ( ii1 == ii2 );
        assert ( ii1 == ii4 );

        assert (!(ii1 != ii2 ));

        assert ( (ii1 == cii ));
        assert ( (cii == ii1 ));
        assert (!(ii1 != cii ));
        assert (!(cii != ii1 ));
    }
//#endif
}

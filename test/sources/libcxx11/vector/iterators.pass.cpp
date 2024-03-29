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

// <vector>

// iterator       begin();
// iterator       end();
// const_iterator begin()  const;
// const_iterator end()    const;
// const_iterator cbegin() const;
// const_iterator cend()   const;

//#include <vector>
//#include <cassert>
//#include <iterator>

//#include "min_allocator.h"

struct A
{
    int first;
    int second;
};

void main()
{
    {
        typedef int T;
        typedef vector<T> C;
        C c;
        C::iterator i = c.begin();
        C::iterator j = c.end();
        assert(std::distance(i, j) == 0);
        assert(i == j);
    }
    {
        typedef int T;
        typedef vector<T> C;
        const C c;
        C::const_iterator i = c.begin();
        C::const_iterator j = c.end();
        assert(std::distance(i, j) == 0);
        assert(i == j);
    }
    {
        typedef int T;
        typedef vector<T> C;
        C c;
        C::const_iterator i = c.cbegin();
        C::const_iterator j = c.cend();
        assert(std::distance(i, j) == 0);
        assert(i == j);
        assert(i == c.end());
    }
    {
        typedef int T;
        typedef vector<T> C;
        C c;
        C::reverse_iterator i = c.rbegin();
        C::reverse_iterator j = c.rend();
        assert(std::distance(i, j) == 0);
        assert(i == j);
    }
    {
        typedef int T;
        typedef vector<T> C;
        const C c;
        C::const_reverse_iterator i = c.rbegin();
        C::const_reverse_iterator j = c.rend();
        assert(std::distance(i, j) == 0);
        assert(i == j);
    }
    {
        typedef int T;
        typedef vector<T> C;
        C c;
        C::const_reverse_iterator i = c.crbegin();
        C::const_reverse_iterator j = c.crend();
        assert(std::distance(i, j) == 0);
        assert(i == j);
        assert(i == c.rend());
    }
    {
        typedef int T;
        typedef vector<T> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t), std::end(t));
        C::iterator i = c.begin();
        assert(*i == 0);
        ++i;
        assert(*i == 1);
        *i = 10;
        assert(*i == 10);
        assert(std::distance(c.begin(), c.end()) == 10);
    }
    {
        typedef int T;
        typedef vector<T> C;
        C::iterator i;
        C::const_iterator j;
		(void)i; (void)j;
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef int T;
        typedef vector<T, min_allocator<T>> C;
        C c;
        C::iterator i = c.begin();
        C::iterator j = c.end();
        assert(std::distance(i, j) == 0);
        assert(i == j);
    }
    {
        typedef int T;
        typedef vector<T, min_allocator<T>> C;
        const C c;
        C::const_iterator i = c.begin();
        C::const_iterator j = c.end();
        assert(std::distance(i, j) == 0);
        assert(i == j);
    }
    {
        typedef int T;
        typedef vector<T, min_allocator<T>> C;
        C c;
        C::const_iterator i = c.cbegin();
        C::const_iterator j = c.cend();
        assert(std::distance(i, j) == 0);
        assert(i == j);
        assert(i == c.end());
    }
    {
        typedef int T;
        typedef vector<T, min_allocator<T>> C;
        const T t[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        C c(std::begin(t), std::end(t));
        C::iterator i = c.begin();
        assert(*i == 0);
        ++i;
        assert(*i == 1);
        *i = 10;
        assert(*i == 10);
        assert(std::distance(c.begin(), c.end()) == 10);
    }
    {
        typedef int T;
        typedef vector<T, min_allocator<T>> C;
        C::iterator i;
        C::const_iterator j;
    }
    {
        typedef A T;
        typedef vector<T, min_allocator<T>> C;
        C c = {A{1, 2}};
        C::iterator i = c.begin();
        i->first = 3;
        C::const_iterator j = i;
        assert(j->first == 3);
    }
#endif
//#if _LIBCPP_STD_VER > 11
    { // N3644 testing
        typedef vector<int> C;
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
        assert (!(ii1 <  cii ));
        assert (!(cii <  ii1 ));
        assert ( (ii1 <= cii ));
        assert ( (cii <= ii1 ));
        assert (!(ii1 >  cii ));
        assert (!(cii >  ii1 ));
        assert ( (ii1 >= cii ));
        assert ( (cii >= ii1 ));
        assert (cii - ii1 == 0);
        assert (ii1 - cii == 0);
    }
//#endif
}

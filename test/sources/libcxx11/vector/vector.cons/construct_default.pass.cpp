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

// vector(const Alloc& = Alloc());

//#include <vector>
//#include <cassert>

//#include "test_allocator.h"
//#include "../../../NotConstructible.h"
//#include "../../../stack_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

template <class C>
void
test0()
{
    C c;
    //assert(c.__invariants());
    assert(c.empty());
    assert(c.get_allocator() == typename C::allocator_type());
    //assert(is_contiguous_container_asan_correct(c));
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    C c1 = {};
    //assert(c1.__invariants());
    assert(c1.empty());
    assert(c1.get_allocator() == typename C::allocator_type());
    //assert(is_contiguous_container_asan_correct(c1));
#endif
}

template <class C>
void
test1(const typename C::allocator_type& a)
{
    C c(a);
    //assert(c.__invariants());
    assert(c.empty());
    assert(c.get_allocator() == a);
    //assert(is_contiguous_container_asan_correct(c));
}

void main()
{
    {
    test0<vector<int> >();
    test0<vector<NotConstructible> >();
    test1<vector<int, test_allocator<int> > >(test_allocator<int>(3));
    test1<vector<NotConstructible, test_allocator<NotConstructible> > >
        (test_allocator<NotConstructible>(5));
    }
#ifdef LIBCPP_TEST_STACK_ALLOCATOR
    {
        vector<int, stack_allocator<int, 10> > v;
        assert(v.empty());
    }
#endif
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    test0<vector<int, min_allocator<int>> >();
    test0<vector<NotConstructible, min_allocator<NotConstructible>> >();
    test1<vector<int, min_allocator<int> > >(min_allocator<int>{});
    test1<vector<NotConstructible, min_allocator<NotConstructible> > >
        (min_allocator<NotConstructible>{});
    }
    {
        vector<int, min_allocator<int> > v;
        assert(v.empty());
    }
#endif
}

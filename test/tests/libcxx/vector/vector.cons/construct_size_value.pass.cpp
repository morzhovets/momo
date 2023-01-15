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

// vector(size_type n, const value_type& x);

//#include <vector>
//#include <cassert>

//#include "../../../stack_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

template <class C>
void
test(typename C::size_type n, const typename C::value_type& x)
{
    C c(n, x);
    //assert(c.__invariants());
    assert(c.size() == n);
    //assert(is_contiguous_container_asan_correct(c));
    for (typename C::const_iterator i = c.cbegin(), e = c.cend(); i != e; ++i)
        assert(*i == x);
}

void main()
{
    test<vector<int> >(50, 3);
#ifdef LIBCPP_TEST_STACK_ALLOCATOR
    test<vector<int, stack_allocator<int, 50> > >(50, 5);
#endif
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    test<vector<int, min_allocator<int>> >(50, 3);
#endif
}

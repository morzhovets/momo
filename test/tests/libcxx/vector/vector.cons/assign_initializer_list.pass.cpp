//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// void assign(initializer_list<value_type> il);

//#include <vector>
//#include <cassert>

//#include "min_allocator.h"
//#include "asan_testing.h"

template <typename Vec>
void test ( Vec &v )
{
    v.assign({3, 4, 5, 6});
    assert(v.size() == 4);
    //assert(is_contiguous_container_asan_correct(v));
    assert(v[0] == 3);
    assert(v[1] == 4);
    assert(v[2] == 5);
    assert(v[3] == 6);
}

void main()
{
#ifndef _LIBCPP_HAS_NO_GENERALIZED_INITIALIZERS
    {
    typedef vector<int> V;
    V d1;
    V d2;
    d2.reserve(10);  // no reallocation during assign.
    test(d1);
    test(d2);
    }

//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef vector<int, min_allocator<int>> V;
    V d1;
    V d2;
    d2.reserve(10);  // no reallocation during assign.
    test(d1);
    test(d2);
    }
#endif
#endif  // _LIBCPP_HAS_NO_GENERALIZED_INITIALIZERS
}

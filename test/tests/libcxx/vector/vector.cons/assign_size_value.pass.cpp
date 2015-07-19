//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// void assign(size_type n, const_reference v);

//#include <vector>
//#include <algorithm>
//#include <cassert>
//#include <iostream>

//#include "min_allocator.h"
//#include "asan_testing.h"

bool is6(int x) { return x == 6; }

template <typename Vec>
void test ( Vec &v )
{
    //std::cout << "Size, cap: " << v.size() << " " << v.capacity() << std::endl;
    v.assign((size_t)5, 6);
    //std::cout << "Size, cap: " << v.size() << " " << v.capacity() << std::endl;
    assert(v.size() == 5);
    //assert(is_contiguous_container_asan_correct(v));
    assert(std::all_of(v.begin(), v.end(), is6));
}

void main()
{
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
}

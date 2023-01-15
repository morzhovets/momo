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

// <vector>

// An vector is a contiguous container

//#include <vector>
//#include <cassert>

//#include "test_macros.h"
//#include "test_allocator.h"
//#include "min_allocator.h"

template <class C>
void test_contiguous ( const C &c )
{
    for ( size_t i = 0; i < c.size(); ++i )
        assert ( *(c.begin() + static_cast<typename C::difference_type>(i)) == *(std::addressof(*c.begin()) + i));
}

void main()
{
    {
    typedef int T;
    typedef vector<T> C;
    test_contiguous(C());
    test_contiguous(C(3, 5));
    }

    {
    typedef double T;
    typedef test_allocator<T> A;
    typedef vector<T, A> C;
    test_contiguous(C(A(3)));
    test_contiguous(C(7, 9.0, A(5)));
    }
//#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef double T;
    typedef min_allocator<T> A;
    typedef vector<T, A> C;
    test_contiguous(C(A{}));
    test_contiguous(C(9, 11.0, A{}));
    }
#endif
}

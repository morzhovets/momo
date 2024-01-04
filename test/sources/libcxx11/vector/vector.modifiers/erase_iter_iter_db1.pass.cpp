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

// Call erase(const_iterator first, const_iterator last); with first iterator from another container

#ifdef LIBCXX_TEST_FAILURE

//#include <vector>
//#include <cassert>
//#include <exception>
//#include <cstdlib>

//#include "min_allocator.h"

void main()
{
    {
    int a1[] = {1, 2, 3};
    vector<int> l1(a1, a1+3);
    vector<int> l2(a1, a1+3);
    LIBCPP_CATCH(l1.erase(l2.cbegin(), l1.cbegin()+1));
    //assert(false);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    int a1[] = {1, 2, 3};
    vector<int, min_allocator<int>> l1(a1, a1+3);
    vector<int, min_allocator<int>> l2(a1, a1+3);
    vector<int, min_allocator<int>>::iterator i = l1.erase(l2.cbegin(), l1.cbegin()+1);
    assert(false);
    }
#endif
}

#else

void main()
{
}

#endif

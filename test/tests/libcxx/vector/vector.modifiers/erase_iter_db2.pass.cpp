//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// Call erase(const_iterator position) with iterator from another container

#if _LIBCPP_DEBUG >= 1

//#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))

//#include <vector>
//#include <cassert>
//#include <cstdlib>
//#include <exception>

//#include "min_allocator.h"

void main()
{
    {
    int a1[] = {1, 2, 3};
    vector<int> l1(a1, a1+3);
    vector<int> l2(a1, a1+3);
    vector<int>::const_iterator i = l2.begin();
    LIBCPP_CATCH(l1.erase(i));
    //assert(false);
    }
#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    int a1[] = {1, 2, 3};
    vector<int, min_allocator<int>> l1(a1, a1+3);
    vector<int, min_allocator<int>> l2(a1, a1+3);
    vector<int, min_allocator<int>>::const_iterator i = l2.begin();
    l1.erase(i);
    assert(false);
    }
#endif
#endif
}

#else

void main()
{
}

#endif

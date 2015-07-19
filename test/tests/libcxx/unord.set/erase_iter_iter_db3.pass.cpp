//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_set>

// Call erase(const_iterator first, const_iterator last); with both iterators from another container

#if _LIBCPP_DEBUG >= 1

#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))

//#include <unordered_set>
//#include <cassert>
//#include <exception>
//#include <cstdlib>

void main()
{
    {
    int a1[] = {1, 2, 3};
    unordered_set<int> l1(a1, a1+3);
    unordered_set<int> l2(a1, a1+3);
    LIBCPP_CATCH(l1.erase(l2.cbegin(), next(l2.cbegin())));
    //assert(false);
    }
}

#else

void main()
{
}

#endif

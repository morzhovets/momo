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

// <unordered_set>

// Call erase(const_iterator first, const_iterator last); with second iterator from another container

#ifdef LIBCXX_TEST_FAILURE

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
    LIBCPP_CATCH(l1.erase(l1.cbegin(), next(l2.cbegin())));
    //assert(false);
    }
}

#else

void main()
{
}

#endif

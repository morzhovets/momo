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

// <unordered_map>

// Call erase(const_iterator first, const_iterator last); with both iterators from another container

#ifdef LIBCXX_TEST_FAILURE

//#include <unordered_map>
//#include <cassert>
//#include <exception>
//#include <cstdlib>

void main()
{
    {
    typedef std::pair<int, int> P;
    P a1[] = {P(1, 1), P(2, 2), P(3, 3)};
    unordered_multimap<int, int> l1(a1, a1+3);
    unordered_multimap<int, int> l2(a1, a1+3);
    LIBCPP_CATCH(l1.erase(l2.cbegin(), next(l2.cbegin())));
    //assert(false);
    }
}

#else

void main()
{
}

#endif

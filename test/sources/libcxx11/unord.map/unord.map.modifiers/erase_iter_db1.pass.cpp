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

// Call erase(const_iterator position) with end()

#ifdef LIBCXX_TEST_FAILURE

//#include <unordered_map>
//#include <cassert>

void main()
{
    {
    typedef std::pair<int, int> P;
    P a1[] = {P(1, 1), P(2, 2), P(3, 3)};
    unordered_map<int, int> l1(a1, a1+3);
    unordered_map<int, int>::const_iterator i = l1.end();
    LIBCPP_CATCH(l1.erase(i));
    //assert(false);
    }
}

#else

void main()
{
}

#endif

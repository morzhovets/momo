//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <unordered_set>

// Call erase(const_iterator position) with end()

#if _LIBCPP_DEBUG >= 1

//#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))

//#include <unordered_set>
//#include <cassert>

void main()
{
    {
    int a1[] = {1, 2, 3};
    unordered_set<int> l1(a1, a1+3);
    unordered_set<int>::const_iterator i = l1.end();
    LIBCPP_CATCH(l1.erase(i));
    //assert(false);
    }
}

#else

void main()
{
}

#endif

//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <vector>

// void pop_back();

#if _LIBCPP_DEBUG >= 1
//#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))
#endif

//#include <vector>
//#include <cassert>
//#include "../../../stack_allocator.h"
//#include "min_allocator.h"

#if _LIBCPP_DEBUG >= 1
//#include <cstdlib>
//#include <exception>

#endif

void main()
{
    {
        vector<int> c;
        c.push_back(1);
        assert(c.size() == 1);
        c.pop_back();
        assert(c.size() == 0);
#if _LIBCPP_DEBUG >= 1
        LIBCPP_CATCH(c.pop_back());
        //assert(false);
#endif
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        vector<int, min_allocator<int>> c;
        c.push_back(1);
        assert(c.size() == 1);
        c.pop_back();
        assert(c.size() == 0);
#if _LIBCPP_DEBUG >= 1
        c.pop_back();
        assert(false);
#endif
    }
#endif
}

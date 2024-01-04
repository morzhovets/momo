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

// void pop_back();

//#include <vector>
//#include <cassert>
//#include "../../../stack_allocator.h"
//#include "min_allocator.h"

void main()
{
    {
        vector<int> c;
        c.push_back(1);
        assert(c.size() == 1);
        c.pop_back();
        assert(c.size() == 0);
#ifdef LIBCXX_TEST_FAILURE
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
#ifdef LIBCXX_TEST_FAILURE
        c.pop_back();
        assert(false);
#endif
    }
#endif
}

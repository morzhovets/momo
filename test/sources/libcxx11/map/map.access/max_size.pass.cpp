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

// <map>

// class map

// size_type max_size() const;

//#include <map>
//#include <cassert>

//#include "min_allocator.h"

void main()
{
    {
    typedef map<int, double> M;
    M m;
    assert(m.max_size() != 0);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef map<int, double, std::less<int>, min_allocator<std::pair<const int, double>>> M;
    M m;
    assert(m.max_size() != 0);
    }
#endif
}

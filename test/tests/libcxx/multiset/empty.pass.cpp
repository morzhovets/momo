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

// <set>

// class multiset

// bool empty() const;

//#include <set>
//#include <cassert>

//#include "min_allocator.h"

void main()
{
    {
    typedef multiset<int> M;
    M m;
    assert(m.empty());
    m.insert(M::value_type(1));
    assert(!m.empty());
    m.clear();
    assert(m.empty());
    }
//#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef multiset<int, std::less<int>, min_allocator<int>> M;
    M m;
    assert(m.empty());
    m.insert(M::value_type(1));
    assert(!m.empty());
    m.clear();
    assert(m.empty());
    }
#endif
}

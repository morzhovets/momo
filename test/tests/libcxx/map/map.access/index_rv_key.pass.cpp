//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// class map

// mapped_type& operator[](key_type&& k);

//#include <map>
//#include <cassert>

//#include "test_macros.h"
//#include "MoveOnly.h"
//#include "min_allocator.h"

void main()
{
//#if TEST_STD_VER >= 11
    {
    map<MoveOnly, double> m;
    assert(m.size() == 0);
#ifndef MOMO_USE_SAFE_MAP_BRACKETS
    assert(m[1] == 0.0);
    assert(m.size() == 1);
#endif
    m[1] = -1.5;
    assert(m[1] == -1.5);
    assert(m.size() == 1);
#ifndef MOMO_USE_SAFE_MAP_BRACKETS
    assert(m[6] == 0);
    assert(m.size() == 2);
#endif
    m[6] = 6.5;
    assert(m[6] == 6.5);
    assert(m.size() == 2);
    }
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef std::pair<const MoveOnly, double> V;
    map<MoveOnly, double, std::less<MoveOnly>, min_allocator<V>> m;
    assert(m.size() == 0);
    assert(m[1] == 0.0);
    assert(m.size() == 1);
    m[1] = -1.5;
    assert(m[1] == -1.5);
    assert(m.size() == 1);
    assert(m[6] == 0);
    assert(m.size() == 2);
    m[6] = 6.5;
    assert(m[6] == 6.5);
    assert(m.size() == 2);
    }
#endif
}

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

// class unordered_map

// size_type size() const noexcept;

//#include <unordered_map>
//#include <cassert>

//#include "test_macros.h"
//#include "min_allocator.h"

void main()
{
    {
    typedef unordered_map<int, double> M;
    M m;
    //ASSERT_NOEXCEPT(m.size());
    assert(m.size() == 0);
    m.insert(M::value_type(2, 1.5));
    assert(m.size() == 1);
    m.insert(M::value_type(1, 1.5));
    assert(m.size() == 2);
    m.insert(M::value_type(3, 1.5));
    assert(m.size() == 3);
    m.erase(m.begin());
    assert(m.size() == 2);
    m.erase(m.begin());
    assert(m.size() == 1);
    m.erase(m.begin());
    assert(m.size() == 0);
    }
//#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    typedef unordered_map<int, double, std::hash<int>, std::equal_to<int>, min_allocator<std::pair<const int, double>>> M;
    M m;
    ASSERT_NOEXCEPT(m.size());
    assert(m.size() == 0);
    m.insert(M::value_type(2, 1.5));
    assert(m.size() == 1);
    m.insert(M::value_type(1, 1.5));
    assert(m.size() == 2);
    m.insert(M::value_type(3, 1.5));
    assert(m.size() == 3);
    m.erase(m.begin());
    assert(m.size() == 2);
    m.erase(m.begin());
    assert(m.size() == 1);
    m.erase(m.begin());
    assert(m.size() == 0);
    }
#endif
}

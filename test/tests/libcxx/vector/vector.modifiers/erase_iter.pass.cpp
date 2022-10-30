//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <vector>

// iterator erase(const_iterator position);

//#include <vector>
//#include <iterator>
//#include <cassert>

//#include "test_macros.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

#ifndef TEST_HAS_NO_EXCEPTIONS
struct Throws {
    Throws() : v_(0) {}
    Throws(int v) : v_(v) {}
    Throws(const Throws  &rhs) : v_(rhs.v_) { if (sThrows) throw 1; }
    Throws(      Throws &&rhs) : v_(rhs.v_) { if (sThrows) throw 1; }
    Throws& operator=(const Throws  &rhs) { v_ = rhs.v_; return *this; }
    Throws& operator=(      Throws &&rhs) { v_ = rhs.v_; return *this; }
    int v_;
    static bool sThrows;
    };

bool Throws::sThrows = false;
#endif

TEST_CONSTEXPR_CXX20 bool tests()
{
    {
    int a1[] = {1, 2, 3, 4, 5};
    vector<int> l1(a1, a1+5);
    l1.erase(l1.begin());
    //assert(is_contiguous_container_asan_correct(l1));
    assert(l1 == vector<int>(a1+1, a1+5));
    }
    {
    int a1[] = {1, 2, 3, 4, 5};
    int e1[] = {1, 3, 4, 5};
    vector<int> l1(a1, a1+5);
    l1.erase(l1.begin() + 1);
    //assert(is_contiguous_container_asan_correct(l1));
    assert(l1 == vector<int>(e1, e1+4));
    }
    {
    int a1[] = {1, 2, 3};
    vector<int> l1(a1, a1+3);
    vector<int>::const_iterator i = l1.begin();
    //assert(is_contiguous_container_asan_correct(l1));
    ++i;
    vector<int>::iterator j = l1.erase(i);
    assert(l1.size() == 2);
    assert(std::distance(l1.begin(), l1.end()) == 2);
    assert(*j == 3);
    assert(*l1.begin() == 1);
    assert(*std::next(l1.begin()) == 3);
    //assert(is_contiguous_container_asan_correct(l1));
    j = l1.erase(j);
    assert(j == l1.end());
    assert(l1.size() == 1);
    assert(std::distance(l1.begin(), l1.end()) == 1);
    assert(*l1.begin() == 1);
    //assert(is_contiguous_container_asan_correct(l1));
    j = l1.erase(l1.begin());
    assert(j == l1.end());
    assert(l1.size() == 0);
    assert(std::distance(l1.begin(), l1.end()) == 0);
    //assert(is_contiguous_container_asan_correct(l1));
    }
//#if TEST_STD_VER >= 11
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
    int a1[] = {1, 2, 3};
    vector<int, min_allocator<int>> l1(a1, a1+3);
    vector<int, min_allocator<int>>::const_iterator i = l1.begin();
    //assert(is_contiguous_container_asan_correct(l1));
    ++i;
    vector<int, min_allocator<int>>::iterator j = l1.erase(i);
    assert(l1.size() == 2);
    assert(std::distance(l1.begin(), l1.end()) == 2);
    assert(*j == 3);
    assert(*l1.begin() == 1);
    assert(*std::next(l1.begin()) == 3);
    //assert(is_contiguous_container_asan_correct(l1));
    j = l1.erase(j);
    assert(j == l1.end());
    assert(l1.size() == 1);
    assert(std::distance(l1.begin(), l1.end()) == 1);
    assert(*l1.begin() == 1);
    //assert(is_contiguous_container_asan_correct(l1));
    j = l1.erase(l1.begin());
    assert(j == l1.end());
    assert(l1.size() == 0);
    assert(std::distance(l1.begin(), l1.end()) == 0);
    //assert(is_contiguous_container_asan_correct(l1));
    }
#endif

    return true;
}

void main()
{
    tests();
//#if TEST_STD_VER > 17
//    static_assert(tests());
//#endif

#ifndef TEST_HAS_NO_EXCEPTIONS
// Test for LWG2853:
// Throws: Nothing unless an exception is thrown by the assignment operator or move assignment operator of T.
    {
        Throws arr[] = {1, 2, 3};
        vector<Throws> v(arr, arr+3);
        Throws::sThrows = true;
        v.erase(v.begin());
#ifdef LIBCXX_TEST_ARRAY
        v.erase(std::prev(v.end()));
#else
        v.erase(--v.end());
#endif
        v.erase(v.begin());
        assert(v.size() == 0);
    }
#endif
}

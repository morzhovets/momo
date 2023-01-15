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

// explicit multiset(const value_compare& comp);
// value_compare and key_compare are the same type for set/multiset

// key_compare    key_comp() const;
// value_compare value_comp() const;

//#include <set>
//#include <cassert>

//#include "../../../test_compare.h"

void main()
{
    typedef test_compare<std::less<int> > C;
    const multiset<int, C> m(C(3));
    assert(m.empty());
    assert(m.begin() == m.end());
    assert(m.key_comp() == C(3));
    assert(m.value_comp() == C(3));
}

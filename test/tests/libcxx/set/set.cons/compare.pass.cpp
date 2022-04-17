//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <set>

// class set

// explicit set(const value_compare& comp);

//#include <set>
//#include <cassert>

//#include "../../../test_compare.h"

void main()
{
    typedef test_compare<std::less<int> > C;
    set<int, C> m(C(3));
    assert(m.empty());
    assert(m.begin() == m.end());
    assert(m.key_comp() == C(3));
}

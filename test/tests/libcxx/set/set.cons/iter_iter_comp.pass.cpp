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

// template <class InputIterator>
//     set(InputIterator first, InputIterator last, const value_compare& comp);

//#include <set>
//#include <cassert>

//#include "test_iterators.h"
//#include "../../../test_compare.h"

void main()
{
    typedef int V;
    V ar[] =
    {
        1,
        1,
        1,
        2,
        2,
        2,
        3,
        3,
        3
    };
    typedef test_compare<std::less<V> > C;
    set<V, C> m(input_iterator<const V*>(ar),
                     input_iterator<const V*>(ar+sizeof(ar)/sizeof(ar[0])), C(5));
    assert(m.value_comp() == C(5));
    assert(m.size() == 3);
    assert(std::distance(m.begin(), m.end()) == 3);
    assert(*m.begin() == 1);
    assert(*std::next(m.begin()) == 2);
    assert(*std::next(m.begin(), 2) == 3);
}

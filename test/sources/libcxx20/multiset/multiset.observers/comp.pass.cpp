//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Modified for https://github.com/morzhovets/momo project.
//
//===----------------------------------------------------------------------===//

// <set>

// key_compare key_comp() const;
// value_compare value_comp() const;

int main(int, char**) {
    typedef std::multiset<int> set_type;

    set_type s;
    set_type::iterator i1 = s.insert(1);
    set_type::iterator i2 = s.insert(2);

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
#else
    i1 = s.find(1);
#endif

    const set_type& cs = s;

    assert(cs.key_comp()(*i1, *i2));
    assert(!cs.key_comp()(*i2, *i1));

    assert(cs.value_comp()(*i1, *i2));
    assert(!cs.value_comp()(*i2, *i1));

    return 0;
}

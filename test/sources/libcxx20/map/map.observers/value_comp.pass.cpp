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

// <map>

// value_compare value_comp() const;

int main(int, char**) {
    typedef std::map<int, std::string> map_type;

    map_type m;
    std::pair<map_type::iterator, bool> p1 = m.insert(map_type::value_type(1, "abc"));
    std::pair<map_type::iterator, bool> p2 = m.insert(map_type::value_type(2, "abc"));

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
#else
    p1.first = m.find(1);
#endif

    const map_type& cm = m;

    assert(cm.value_comp()(*p1.first, *p2.first));
    assert(!cm.value_comp()(*p2.first, *p1.first));

    return 0;
}

//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <unordered_set>

// class unordered_set

// iterator insert(const_iterator hint, node_type&&);

//#include <unordered_set>
//#include "min_allocator.h"

template <class Container>
typename Container::node_type
node_factory(typename Container::key_type const& key)
{
    static Container c;
    auto p = c.insert(key);
    assert(p.second);
    return c.extract(p.first);
}

template <class Container>
void test(Container& c)
{
    auto* nf = &node_factory<Container>;

    for (int i = 0; i != 10; ++i)
    {
        typename Container::node_type node = nf(i);
        assert(!node.empty());
        size_t prev = c.size();
        auto it = c.insert(c.end(), std::move(node));
        assert(node.empty());
        assert(prev + 1 == c.size());
        assert(*it == i);
    }

    assert(c.size() == 10);

    for (int i = 0; i != 10; ++i)
    {
        assert(c.count(i) == 1);
    }
}

void main()
{
    unordered_set<int> m;
    test(m);
    unordered_set<int, std::hash<int>, std::equal_to<int>, min_allocator<int>> m2;
    test(m2);
}

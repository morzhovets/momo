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

// UNSUPPORTED: c++98, c++03, c++11, c++14

// <map>

// class multimap

// iterator insert(const_iterator hint, node_type&&);

//#include <map>
//#include "min_allocator.h"

template <class Container>
typename Container::node_type
node_factory(typename Container::key_type const& key,
             typename Container::mapped_type const& mapped)
{
    static Container c;
    auto it = c.insert({key, mapped});
    return c.extract(it);
}

template <class Container>
void test(Container& c)
{
    auto* nf = &node_factory<Container>;

    for (int i = 0; i != 10; ++i)
    {
        typename Container::node_type node = nf(i, i + 1);
        assert(!node.empty());
        size_t prev = c.size();
        auto it = c.insert(c.end(), std::move(node));
        assert(node.empty());
        assert(prev + 1 == c.size());
        assert(it == c.find(i));
        assert(it->first == i);
        assert(it->second == i + 1);
    }

    assert(c.size() == 10);

    for (int i = 0; i != 10; ++i)
    {
        assert(c.count(i) == 1);
        assert(c.find(i)->second == i + 1);
    }
}

void main()
{
    multimap<int, int> m;
    test(m);
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    multimap<int, int, std::less<int>, min_allocator<std::pair<const int, int>>> m2;
    test(m2);
#endif
}

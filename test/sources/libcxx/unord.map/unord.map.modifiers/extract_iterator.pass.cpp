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

// <unordered_map>

// class unordered_map

// node_type extract(const_iterator);

//#include <unordered_map>
//#include "min_allocator.h"
//#include "Counter.h"

template <class Container>
void test(Container& c)
{
    size_t sz = c.size();

    auto some_key = c.cbegin()->first;

    for (auto first = c.cbegin(); first != c.cend();)
    {
        auto key_value = first->first;
        typename Container::node_type t = c.extract(first++);
        --sz;
        assert(t.key() == key_value);
        t.key() = some_key;
        assert(t.key() == some_key);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(t.get_allocator() == c.get_allocator());
#endif
        assert(sz == c.size());
    }

    assert(c.size() == 0);
}

void main()
{
    {
        using map_type = unordered_map<int, int>;
        map_type m = {{1,1}, {2,2}, {3,3}, {4,4}, {5,5}, {6,6}};
        test(m);
    }

    {
        unordered_map<Counter<int>, Counter<int>> m =
            {{1,1}, {2,2}, {3,3}, {4,4}, {5,5}, {6,6}};
        assert(Counter_base::gConstructed == 12);
        test(m);
        assert(Counter_base::gConstructed == 0);
    }
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        using min_alloc_map =
            unordered_map<int, int, std::hash<int>, std::equal_to<int>,
                               min_allocator<std::pair<const int, int>>>;
        min_alloc_map m = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}};
        test(m);
    }
#endif
}

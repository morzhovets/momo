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

// UNSUPPORTED: c++03, c++11, c++14

// <unordered_map>

// class unordered_map

// node_type extract(const_iterator);

template <class Container>
void test(Container& c)
{
    std::size_t sz = c.size();

    auto some_key = c.cbegin()->first;

#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    for (auto first = c.cbegin(); first != c.cend();)
    {
        auto key_value = first->first;
        typename Container::node_type t = c.extract(first++);
        --sz;
        assert(t.key() == key_value);
        t.key() = some_key;
        assert(t.key() == some_key);
        assert(t.get_allocator() == c.get_allocator());
        assert(sz == c.size());
    }
#else
    for (auto first = c.cbegin(); first != c.cend(); first = c.cbegin())
    {
        auto key_value = first->first;
        typename Container::node_type t = c.extract(first);
        --sz;
        assert(t.key() == key_value);
        t.key() = some_key;
        assert(t.key() == some_key);
        assert(sz == c.size());
    }
#endif

    assert(c.size() == 0);
}

int main(int, char**)
{
    {
        using map_type = std::unordered_map<int, int>;
        map_type m = {{1,1}, {2,2}, {3,3}, {4,4}, {5,5}, {6,6}};
        test(m);
    }

    {
        std::unordered_map<Counter<int>, Counter<int>> m =
            {{1,1}, {2,2}, {3,3}, {4,4}, {5,5}, {6,6}};
        assert(Counter_base::gConstructed == 12);
        test(m);
        assert(Counter_base::gConstructed == 0);
    }

    {
        using min_alloc_map =
            std::unordered_map<int, int, std::hash<int>, std::equal_to<int>,
                               min_allocator<std::pair<const int, int>>>;
        min_alloc_map m = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}};
        test(m);
    }

  return 0;
}

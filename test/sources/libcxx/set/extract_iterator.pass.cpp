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

// <set>

// class set

// node_type extract(const_iterator);

template <class Container>
void test(Container& c)
{
    size_t sz = c.size();

    for (auto first = c.cbegin(); first != c.cend();)
    {
        auto key_value = *first;
        typename Container::node_type t = c.extract(first++);
        --sz;
        assert(t.value() == key_value);
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
        using set_type = set<int>;
        set_type m = {1, 2, 3, 4, 5, 6};
        test(m);
    }

    {
        set<Counter<int>> m = {1, 2, 3, 4, 5, 6};
        assert(Counter_base::gConstructed == 6);
        test(m);
        assert(Counter_base::gConstructed == 0);
    }
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        using min_alloc_set = set<int, std::less<int>, min_allocator<int>>;
        min_alloc_set m = {1, 2, 3, 4, 5, 6};
        test(m);
    }
#endif
}

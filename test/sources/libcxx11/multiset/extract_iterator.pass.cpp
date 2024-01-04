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

// class multiset

// node_type extract(const_iterator);

//#include <set>
//#include "min_allocator.h"
//#include "Counter.h"

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
        assert(t.get_allocator() == c.get_allocator());
        assert(sz == c.size());
    }

    assert(c.size() == 0);
}

void main()
{
    {
        using set_type = multiset<int>;
        set_type m = {1, 2, 3, 4, 5, 6};
        test(m);
    }

    {
        multiset<Counter<int>> m = {1, 2, 3, 4, 5, 6};
        assert(Counter_base::gConstructed == 6);
        test(m);
        assert(Counter_base::gConstructed == 0);
    }
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        using min_alloc_set = multiset<int, std::less<int>, min_allocator<int>>;
        min_alloc_set m = {1, 2, 3, 4, 5, 6};
        test(m);
    }
#endif
}

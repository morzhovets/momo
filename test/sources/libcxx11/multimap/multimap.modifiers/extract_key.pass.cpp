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

// node_type extract(key_type const&);

//#include <map>
//#include "min_allocator.h"
//#include "Counter.h"

template <class Container, class KeyTypeIter>
void test(Container& c, KeyTypeIter first, KeyTypeIter last)
{
    size_t sz = c.size();
    assert(momo::internal::UIntMath<>::Dist(first, last) == sz);

    for (KeyTypeIter copy = first; copy != last; ++copy)
    {
        typename Container::node_type t = c.extract(*copy);
        assert(!t.empty());
        --sz;
        assert(t.key() == *copy);
        t.key() = *first; // We should be able to mutate key.
        assert(t.key() == *first);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(t.get_allocator() == c.get_allocator());
#endif
        assert(sz == c.size());
    }

    assert(c.size() == 0);

    for (KeyTypeIter copy = first; copy != last; ++copy)
    {
        typename Container::node_type t = c.extract(*copy);
        assert(t.empty());
    }
}

void main()
{
    {
        multimap<int, int> m = {{1,1}, {2,2}, {3,3}, {4,4}, {5,5}, {6,6}};
        int keys[] = {1, 2, 3, 4, 5, 6};
        test(m, std::begin(keys), std::end(keys));
    }

    {
        multimap<Counter<int>, Counter<int>> m =
            {{1,1}, {2,2}, {3,3}, {4,4}, {5,5}, {6,6}};
        {
            Counter<int> keys[] = {1, 2, 3, 4, 5, 6};
            assert(Counter_base::gConstructed == 12+6);
            test(m, std::begin(keys), std::end(keys));
        }
        assert(Counter_base::gConstructed == 0);
    }
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        using min_alloc_map =
            multimap<int, int, std::less<int>,
                          min_allocator<std::pair<const int, int>>>;
        min_alloc_map m = {{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}};
        int keys[] = {1, 2, 3, 4, 5, 6};
        test(m, std::begin(keys), std::end(keys));
    }
#endif
}

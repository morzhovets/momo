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

// template <class C2>
//   void merge(set<Key, C2, Allocator>& source);
// template <class C2>
//   void merge(set<Key, C2, Allocator>&& source);
// template <class C2>
//   void merge(multiset<Key, C2, Allocator>& source);
// template <class C2>
//   void merge(multiset<Key, C2, Allocator>&& source);

//#include <set>
//#include "test_macros.h"
//#include "Counter.h"

using momo::stdish::set;

template <class Set>
bool set_equal(const Set& set, Set other)
{
    return set == other;
}

#ifndef TEST_HAS_NO_EXCEPTIONS
struct throw_comparator
{
    bool& should_throw_;

    throw_comparator(bool& should_throw) : should_throw_(should_throw) {}

    template <class T>
    bool operator()(const T& lhs, const T& rhs) const
    {
        if (should_throw_)
            throw 0;
        return lhs < rhs;
    }
};
#endif

void main()
{
    {
        multiset<int> src{1, 3, 5};
        multiset<int> dst{2, 4, 5};
        dst.merge(src);
        assert(set_equal(src, {}));
        assert(set_equal(dst, {1, 2, 3, 4, 5, 5}));
    }

#ifndef TEST_HAS_NO_EXCEPTIONS
    {
        bool do_throw = false;
        typedef multiset<Counter<int>, throw_comparator> set_type;
        set_type src({1, 3, 5}, throw_comparator(do_throw));
        set_type dst({2, 4, 5}, throw_comparator(do_throw));

        assert(Counter_base::gConstructed == 6);

        do_throw = true;
        try
        {
            dst.merge(src);
        }
        catch (int)
        {
            do_throw = false;
        }
        assert(!do_throw);
        assert(set_equal(src, set_type({1, 3, 5}, throw_comparator(do_throw))));
        assert(set_equal(dst, set_type({2, 4, 5}, throw_comparator(do_throw))));
    }
#endif
    assert(Counter_base::gConstructed == 0);
    struct comparator
    {
        comparator() = default;

        bool operator()(const Counter<int>& lhs, const Counter<int>& rhs) const
        {
            return lhs < rhs;
        }
    };
    {
        typedef multiset<Counter<int>, std::less<Counter<int>>> first_set_type;
        typedef multiset<Counter<int>, comparator> second_set_type;
        typedef set<Counter<int>, comparator> third_set_type;

        {
            first_set_type first{1, 2, 3};
            second_set_type second{2, 3, 4};
            third_set_type third{1, 3};

            assert(Counter_base::gConstructed == 8);

            first.merge(second);
            first.merge(third);

            assert(set_equal(first, {1, 1, 2, 2, 3, 3, 3, 4}));
            assert(set_equal(second, {}));
            assert(set_equal(third, {}));

            assert(Counter_base::gConstructed == 8);
        }
        assert(Counter_base::gConstructed == 0);
        {
            first_set_type first{1, 2, 3};
            second_set_type second{2, 3, 4};
            third_set_type third{1, 3};

            assert(Counter_base::gConstructed == 8);

            first.merge(std::move(second));
            first.merge(std::move(third));

            assert(set_equal(first, {1, 1, 2, 2, 3, 3, 3, 4}));
            assert(set_equal(second, {}));
            assert(set_equal(third, {}));

            assert(Counter_base::gConstructed == 8);
        }
        assert(Counter_base::gConstructed == 0);
    }
    {
        multiset<int> first;
        {
            multiset<int> second;
            first.merge(second);
            first.merge(std::move(second));
        }
        {
            set<int> second;
            first.merge(second);
            first.merge(std::move(second));
        }
    }
}

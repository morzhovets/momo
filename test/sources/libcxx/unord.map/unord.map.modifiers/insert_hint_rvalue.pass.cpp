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

// UNSUPPORTED: c++03

// <unordered_map>

// template <class Key, class T, class Hash = hash<Key>, class Pred = equal_to<Key>,
//           class Alloc = allocator<pair<const Key, T>>>
// class unordered_map

// template <class P,
//           class = typename enable_if<is_convertible<P, value_type>::value>::type>
//     iterator insert(const_iterator p, P&& x);

int main(int, char**)
{
    {
        typedef std::unordered_map<double, int> C;
        typedef C::iterator R;
        typedef std::pair<double, short> P;
        C c;
        C::const_iterator e = /*c.end()*/c.find(3.5);
        R r = c.insert(e, P(3.5, static_cast<short>(3)));
        assert(c.size() == 1);
        assert(r->first == 3.5);
        assert(r->second == 3);

#ifndef LIBCXX_TEST_HINT_ITERATORS
        r = c.insert(/*r*/c.find(3.5), P(3.5, static_cast<short>(4)));
        assert(c.size() == 1);
        assert(r->first == 3.5);
        assert(r->second == 3);
#endif


        r = c.insert(/*c.end()*/c.find(4.5), P(4.5, static_cast<short>(4)));
        assert(c.size() == 2);
        assert(r->first == 4.5);
        assert(r->second == 4);

        r = c.insert(/*c.end()*/c.find(5.5), P(5.5, static_cast<short>(4)));
        assert(c.size() == 3);
        assert(r->first == 5.5);
        assert(r->second == 4);
    }
    {
        typedef std::unordered_map<MoveOnly, MoveOnly> C;
        typedef C::iterator R;
        typedef std::pair<MoveOnly, MoveOnly> P;
        C c;
        C::const_iterator e = /*c.end()*/c.find(3);
        R r = c.insert(e, P(3, 3));
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == 3);

#ifndef LIBCXX_TEST_HINT_ITERATORS
        r = c.insert(/*r*/c.find(3), P(3, 4));
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == 3);
#endif

        r = c.insert(/*c.end()*/c.find(4), P(4, 4));
        assert(c.size() == 2);
        assert(r->first == 4);
        assert(r->second == 4);

        r = c.insert(/*c.end()*/c.find(5), P(5, 4));
        assert(c.size() == 3);
        assert(r->first == 5);
        assert(r->second == 4);
    }
    {
        typedef std::unordered_map<double, int, std::hash<double>, std::equal_to<double>,
                            min_allocator<std::pair<const double, int>>> C;
        typedef C::iterator R;
        typedef std::pair<double, short> P;
        C c;
        C::const_iterator e = /*c.end()*/c.find(3.5);
        R r = c.insert(e, P(3.5, static_cast<short>(3)));
        assert(c.size() == 1);
        assert(r->first == 3.5);
        assert(r->second == 3);

#ifndef LIBCXX_TEST_HINT_ITERATORS
        r = c.insert(/*r*/c.find(3.5), P(3.5, static_cast<short>(4)));
        assert(c.size() == 1);
        assert(r->first == 3.5);
        assert(r->second == 3);
#endif

        r = c.insert(/*c.end()*/c.find(4.5), P(4.5, static_cast<short>(4)));
        assert(c.size() == 2);
        assert(r->first == 4.5);
        assert(r->second == 4);

        r = c.insert(/*c.end()*/c.find(5.5), P(5.5, static_cast<short>(4)));
        assert(c.size() == 3);
        assert(r->first == 5.5);
        assert(r->second == 4);
    }
    {
        typedef std::unordered_map<MoveOnly, MoveOnly, std::hash<MoveOnly>, std::equal_to<MoveOnly>,
                            min_allocator<std::pair<const MoveOnly, MoveOnly>>> C;
        typedef C::iterator R;
        typedef std::pair<MoveOnly, MoveOnly> P;
        C c;
        C::const_iterator e = /*c.end()*/c.find(3);
        R r = c.insert(e, P(3, 3));
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == 3);

#ifndef LIBCXX_TEST_HINT_ITERATORS
        r = c.insert(/*r*/c.find(3), P(3, 4));
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == 3);
#endif

        r = c.insert(/*c.end()*/c.find(4), P(4, 4));
        assert(c.size() == 2);
        assert(r->first == 4);
        assert(r->second == 4);

        r = c.insert(/*c.end()*/c.find(5), P(5, 4));
        assert(c.size() == 3);
        assert(r->first == 5);
        assert(r->second == 4);
    }
    {
        typedef std::unordered_map<double, MoveOnly> C;
        typedef C::iterator R;
        C c;
        C::const_iterator e = /*c.end()*/c.find(3.5);
        R r = c.insert(e, {3.5, 3});
        assert(c.size() == 1);
        assert(r->first == 3.5);
        assert(r->second == 3);

#ifndef LIBCXX_TEST_HINT_ITERATORS
        r = c.insert(/*r*/c.find(3.5), {3.5, 4});
        assert(c.size() == 1);
        assert(r->first == 3.5);
        assert(r->second == 3);
#endif

        r = c.insert(/*c.end()*/c.find(4.5), {4.5, 4});
        assert(c.size() == 2);
        assert(r->first == 4.5);
        assert(r->second == 4);

        r = c.insert(/*c.end()*/c.find(5.5), {5.5, 4});
        assert(c.size() == 3);
        assert(r->first == 5.5);
        assert(r->second == 4);
    }

    return 0;
}

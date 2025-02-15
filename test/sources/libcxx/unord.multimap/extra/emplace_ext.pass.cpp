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
// class unordered_multimap

// template <class... Args>
//     iterator emplace(Args&&... args);
// template <class... Args>
//     iterator emplace_hint(const_iterator p, Args&&... args);

void main()
{
    {
        typedef std::unordered_multimap<int, Emplaceable> C;
        typedef C::iterator R;
        C c;
        R r = c.emplace(std::piecewise_construct, std::forward_as_tuple(3),
                                                  std::forward_as_tuple());
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == Emplaceable());

        r = c.emplace(std::pair<const int, Emplaceable>(4, Emplaceable(5, 6)));
        assert(c.size() == 2);
        assert(r->first == 4);
        assert(r->second == Emplaceable(5, 6));

        r = c.emplace(5, Emplaceable(6, 7));
        assert(c.size() == 3);
        assert(r->first == 5);
        assert(r->second == Emplaceable(6, 7));

        r = c.emplace();
        assert(c.size() == 4);
        assert(r->first == 0);
        assert(r->second == Emplaceable());

        r = c.emplace(std::make_tuple(0, Emplaceable(7, 8)));
        assert(c.size() == 5);
        assert(r->first == 0);
        assert(r->second == Emplaceable(7, 8));
    }
    {
        typedef std::unordered_multimap<std::string, double> C;
        typedef C::iterator R;
        C c;
        R r = c.emplace(std::piecewise_construct, std::forward_as_tuple(2, 'a'),
                                                  std::forward_as_tuple(3.5));
        assert(r == c.begin());
        assert(c.size() == 1);
        assert(c.begin()->first == "aa");
        assert(c.begin()->second == 3.5);
    }
    {
        typedef std::unordered_multimap<int, Emplaceable> C;
        typedef C::iterator R;
        C c;
        R r = c.emplace_hint(c.end(), std::piecewise_construct, std::forward_as_tuple(3),
                                                          std::forward_as_tuple());
        assert(c.size() == 1);
        assert(r->first == 3);
        assert(r->second == Emplaceable());

        r = c.emplace_hint(c.end(), std::pair<const int, Emplaceable>(3, Emplaceable(5, 6)));
        assert(c.size() == 2);
        assert(r->first == 3);
        assert(r->second == Emplaceable(5, 6));
        assert(r == next(c.begin()));

        r = c.emplace_hint(c.end(), 3, Emplaceable(6, 7));
        assert(c.size() == 3);
        assert(r->first == 3);
        assert(r->second == Emplaceable(6, 7));
        r = c.begin();
        assert(r->first == 3);
        assert(r->second == Emplaceable());
        r = next(r, 2);
        assert(r->first == 3);

        r = c.emplace_hint(c.end());
        assert(c.size() == 4);
        assert(r->first == 0);
        assert(r->second == Emplaceable());

        r = c.emplace_hint(c.end(), std::make_tuple(0, Emplaceable(7, 8)));
        assert(c.size() == 5);
        assert(r->first == 0);
        assert(r->second == Emplaceable(7, 8));
    }
}

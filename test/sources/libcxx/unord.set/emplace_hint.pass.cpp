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

// <unordered_set>

// template <class Value, class Hash = hash<Value>, class Pred = equal_to<Value>,
//           class Alloc = allocator<Value>>
// class unordered_set

// template <class... Args>
//     iterator emplace_hint(const_iterator p, Args&&... args);


int main(int, char**)
{
    {
        typedef std::unordered_set<Emplaceable> C;
        typedef C::iterator R;
        C c;
        R r = c.emplace_hint(/*c.end()*/c.find(Emplaceable()));
        assert(c.size() == 1);
        assert(*r == Emplaceable());

        r = c.emplace_hint(/*c.end()*/c.find(Emplaceable(5, 6)), Emplaceable(5, 6));
        assert(c.size() == 2);
        assert(*r == Emplaceable(5, 6));

#ifndef LIBCXX_TEST_HINT_ITERATORS
        r = c.emplace_hint(r, 5, 6);
        assert(c.size() == 2);
        assert(*r == Emplaceable(5, 6));
#endif
    }
    {
        typedef std::unordered_set<Emplaceable, std::hash<Emplaceable>,
                      std::equal_to<Emplaceable>, min_allocator<Emplaceable>> C;
        typedef C::iterator R;
        C c;
        R r = c.emplace_hint(/*c.end()*/c.find(Emplaceable()));
        assert(c.size() == 1);
        assert(*r == Emplaceable());

        r = c.emplace_hint(/*c.end()*/c.find(Emplaceable(5, 6)), Emplaceable(5, 6));
        assert(c.size() == 2);
        assert(*r == Emplaceable(5, 6));

#ifndef LIBCXX_TEST_HINT_ITERATORS
        r = c.emplace_hint(r, 5, 6);
        assert(c.size() == 2);
        assert(*r == Emplaceable(5, 6));
#endif
    }

  return 0;
}

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

// <unordered_set>

// template <class Value, class Hash = hash<Value>, class Pred = equal_to<Value>,
//           class Alloc = allocator<Value>>
// class unordered_set

// template <class... Args>
//     pair<iterator, bool> emplace(Args&&... args);

//#include <unordered_set>
//#include <cassert>

//#include "../../Emplaceable.h"
//#include "min_allocator.h"

void main()
{
#ifndef _LIBCPP_HAS_NO_VARIADICS
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
    {
        typedef unordered_set<Emplaceable> C;
        typedef std::pair<C::iterator, bool> R;
        C c;
        R r = c.emplace();
        assert(c.size() == 1);
        assert(*r.first == Emplaceable());
        assert(r.second);

        r = c.emplace(Emplaceable(5, 6));
        assert(c.size() == 2);
        assert(*r.first == Emplaceable(5, 6));
        assert(r.second);

        r = c.emplace(5, 6);
        assert(c.size() == 2);
        assert(*r.first == Emplaceable(5, 6));
        assert(!r.second);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef unordered_set<Emplaceable, std::hash<Emplaceable>,
                      std::equal_to<Emplaceable>, min_allocator<Emplaceable>> C;
        typedef std::pair<C::iterator, bool> R;
        C c;
        R r = c.emplace();
        assert(c.size() == 1);
        assert(*r.first == Emplaceable());
        assert(r.second);

        r = c.emplace(Emplaceable(5, 6));
        assert(c.size() == 2);
        assert(*r.first == Emplaceable(5, 6));
        assert(r.second);

        r = c.emplace(5, 6);
        assert(c.size() == 2);
        assert(*r.first == Emplaceable(5, 6));
        assert(!r.second);
    }
#endif
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
#endif  // _LIBCPP_HAS_NO_VARIADICS
}

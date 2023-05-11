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

// <unordered_map>

// template <class Key, class T, class Hash = hash<Key>, class Pred = equal_to<Key>,
//           class Alloc = allocator<pair<const Key, T>>>
// class unordered_multimap

// float max_load_factor() const;
// void max_load_factor(float mlf);

#ifdef _LIBCPP_DEBUG
//#define _LIBCPP_ASSERT(x, m) ((x) ? (void)0 : std::exit(0))
#endif

//#include <unordered_map>
//#include <string>
//#include <cassert>

//#include "min_allocator.h"

void main()
{
    {
        typedef unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        const C c;
        assert(c.max_load_factor() == 1);
    }
    {
        typedef unordered_multimap<int, std::string> C;
        typedef std::pair<int, std::string> P;
        C c;
        assert(c.max_load_factor() == 1);
        c.max_load_factor(2.5);
        assert(c.max_load_factor() == 2.5);
    }
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    {
        typedef unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
                            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        const C c;
        assert(c.max_load_factor() == 1);
    }
    {
        typedef unordered_multimap<int, std::string, std::hash<int>, std::equal_to<int>,
                            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        C c;
        assert(c.max_load_factor() == 1);
        c.max_load_factor(2.5);
        assert(c.max_load_factor() == 2.5);
    }
#endif
#if _LIBCPP_DEBUG_LEVEL >= 1
    {
        typedef unordered_multimap<int, std::string> C;
        C c;
        c.max_load_factor(0);
        assert(false);
    }
#endif
}
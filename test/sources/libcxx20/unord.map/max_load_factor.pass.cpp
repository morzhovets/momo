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

// <unordered_map>

// template <class Key, class T, class Hash = hash<Key>, class Pred = equal_to<Key>,
//           class Alloc = allocator<pair<const Key, T>>>
// class unordered_map

// float max_load_factor() const;
// void max_load_factor(float mlf);

int main(int, char**)
{
    {
        typedef std::unordered_map<int, std::string> C;
        const C c;
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c.max_load_factor() == 1);
#endif
    }
    {
        typedef std::unordered_map<int, std::string> C;
        C c;
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c.max_load_factor() == 1);
        c.max_load_factor(2.5);
        assert(c.max_load_factor() == 2.5);
#else
        c.max_load_factor(0.5);
        assert(c.max_load_factor() == 0.5);
#endif
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
                            min_allocator<std::pair<const int, std::string>>> C;
        const C c;
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c.max_load_factor() == 1);
#endif
    }
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
                            min_allocator<std::pair<const int, std::string>>> C;
        C c;
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c.max_load_factor() == 1);
        c.max_load_factor(2.5);
        assert(c.max_load_factor() == 2.5);
#else
        c.max_load_factor(0.5);
        assert(c.max_load_factor() == 0.5);
#endif
    }
#endif

    return 0;
}

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

// UNSUPPORTED: c++03, c++11

// <unordered_map>

// template <class Key, class T, class Hash = hash<Key>, class Pred = equal_to<Key>,
//           class Alloc = allocator<pair<const Key, T>>>
// class unordered_map

// unordered_map(size_type n, const allocator_type& alloc);

template <class Allocator>
void test(const Allocator& alloc) {
    typedef std::unordered_map<NotConstructible, NotConstructible,
                               test_hash<NotConstructible>,
                               test_equal_to<NotConstructible>,
                               Allocator
                               > C;

    C c(7, alloc);
    LIBCPP_ASSERT(c.bucket_count() == 7);
    assert(c.hash_function() == test_hash<NotConstructible>());
    assert(c.key_eq() == test_equal_to<NotConstructible>());
    assert(c.get_allocator() == alloc);
    assert(c.size() == 0);
    assert(c.empty());
    assert(std::distance(c.begin(), c.end()) == 0);
    assert(c.load_factor() == 0);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(c.max_load_factor() == 1);
#endif
}

int main(int, char**)
{
    typedef std::pair<const NotConstructible, NotConstructible> V;

    test(test_allocator<V>(10));
    test(min_allocator<V>());
    test(explicit_allocator<V>());

    return 0;
}

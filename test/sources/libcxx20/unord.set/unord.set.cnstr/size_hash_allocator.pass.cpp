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

// <unordered_set>

// template <class Value, class Hash = hash<Value>, class Pred = equal_to<Value>,
//           class Alloc = allocator<Value>>
// class unordered_set

// unordered_set(size_type n, const hasher& hash, const allocator_type& alloc);

template <class Allocator>
void test(const Allocator& alloc)
{
    typedef std::unordered_set<NotConstructible,
                               test_hash<NotConstructible>,
                               test_equal_to<NotConstructible>,
                               Allocator> C;
    C c(7, test_hash<NotConstructible>(5), alloc);
    LIBCPP_ASSERT(c.bucket_count() == 7);
    assert(c.hash_function() == test_hash<NotConstructible>(5));
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
    test(test_allocator<NotConstructible>(10));
    test(min_allocator<NotConstructible>());
    test(explicit_allocator<NotConstructible>());

    return 0;
}

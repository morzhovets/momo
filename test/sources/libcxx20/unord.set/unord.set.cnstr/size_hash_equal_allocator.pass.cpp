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

// <unordered_set>

// template <class Value, class Hash = hash<Value>, class Pred = equal_to<Value>,
//           class Alloc = allocator<Value>>
// class unordered_set

// unordered_set(size_type n, const hasher& hf, const key_equal& eql, const allocator_type& a);

int main(int, char**)
{
    {
        typedef std::unordered_set<NotConstructible,
                                   test_hash<NotConstructible>,
                                   test_equal_to<NotConstructible>,
                                   test_allocator<NotConstructible>
                                   > C;
        C c(7,
            test_hash<NotConstructible>(8),
            test_equal_to<NotConstructible>(9),
            test_allocator<NotConstructible>(10)
           );
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.hash_function() == test_hash<NotConstructible>(8));
        assert(c.key_eq() == test_equal_to<NotConstructible>(9));
        assert(c.get_allocator() == (test_allocator<NotConstructible>(10)));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c.max_load_factor() == 1);
#endif
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_set<NotConstructible,
                                   test_hash<NotConstructible>,
                                   test_equal_to<NotConstructible>,
                                   min_allocator<NotConstructible>
                                   > C;
        C c(7,
            test_hash<NotConstructible>(8),
            test_equal_to<NotConstructible>(9),
            min_allocator<NotConstructible>()
           );
        LIBCPP_ASSERT(c.bucket_count() == 7);
        assert(c.hash_function() == test_hash<NotConstructible>(8));
        assert(c.key_eq() == test_equal_to<NotConstructible>(9));
        assert(c.get_allocator() == (min_allocator<NotConstructible>()));
        assert(c.size() == 0);
        assert(c.empty());
        assert(std::distance(c.begin(), c.end()) == 0);
        assert(c.load_factor() == 0);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        assert(c.max_load_factor() == 1);
#endif
    }
#endif

  return 0;
}

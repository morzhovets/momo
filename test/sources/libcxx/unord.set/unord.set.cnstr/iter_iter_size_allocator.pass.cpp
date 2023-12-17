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

// template <class InputIterator>
//     unordered_set(InputIterator first, InputIterator last, size_type n,
//                   const allocator_type& alloc);

template <class Allocator>
void test(const Allocator& alloc)
{
    typedef std::unordered_set<int,
                               test_hash<int>,
                               test_equal_to<int>,
                               Allocator> C;
    int a[] =
    {
        1,
        2,
        3,
        4,
        1,
        2
    };
    C c(cpp17_input_iterator<int*>(a), cpp17_input_iterator<int*>(a + sizeof(a)/sizeof(a[0])),
        7,
        alloc
        );
    LIBCPP_ASSERT(c.bucket_count() == 7);
    assert(c.size() == 4);
    assert(c.count(1) == 1);
    assert(c.count(2) == 1);
    assert(c.count(3) == 1);
    assert(c.count(4) == 1);
    assert(c.hash_function() == test_hash<int>());
    assert(c.key_eq() == test_equal_to<int>());
    assert(c.get_allocator() == alloc);
    assert(!c.empty());
    assert(static_cast<std::size_t>(std::distance(c.begin(), c.end())) == c.size());
    assert(std::fabs(c.load_factor() - static_cast<float>(c.size())/c.bucket_count()) < FLT_EPSILON);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(c.max_load_factor() == 1);
#endif
}

int main(int, char**)
{
    test(test_allocator<int>(10));
    test(min_allocator<int>());
    test(explicit_allocator<int>());

    return 0;
}

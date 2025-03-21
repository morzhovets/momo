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

// <vector>

// template <class InputIter> vector(InputIter first, InputIter last);

//#include <vector>
//#include <cassert>

//#include "test_iterators.h"
//#include "../../../stack_allocator.h"
//#include "min_allocator.h"
//#include "asan_testing.h"

template <class C, class Iterator>
void
test(Iterator first, Iterator last)
{
    C c(first, last);
    //assert(c.__invariants());
    assert(c.size() == static_cast<size_t>(std::distance(first, last)));
    //assert(is_contiguous_container_asan_correct(c));
    for (typename C::const_iterator i = c.cbegin(), e = c.cend(); i != e; ++i, ++first)
        assert(*i == *first);
}

void main()
{
    int a[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 8, 7, 6, 5, 4, 3, 1, 0};
    int* an = a + sizeof(a)/sizeof(a[0]);
    test<vector<int> >(input_iterator<const int*>(a), input_iterator<const int*>(an));
    test<vector<int> >(forward_iterator<const int*>(a), forward_iterator<const int*>(an));
    test<vector<int> >(bidirectional_iterator<const int*>(a), bidirectional_iterator<const int*>(an));
    test<vector<int> >(random_access_iterator<const int*>(a), random_access_iterator<const int*>(an));
    test<vector<int> >(a, an);

#ifdef LIBCPP_TEST_STACK_ALLOCATOR
    test<vector<int, stack_allocator<int, 63> > >(input_iterator<const int*>(a), input_iterator<const int*>(an));
    test<vector<int, stack_allocator<int, 18> > >(forward_iterator<const int*>(a), forward_iterator<const int*>(an));
    test<vector<int, stack_allocator<int, 18> > >(bidirectional_iterator<const int*>(a), bidirectional_iterator<const int*>(an));
    test<vector<int, stack_allocator<int, 18> > >(random_access_iterator<const int*>(a), random_access_iterator<const int*>(an));
    test<vector<int, stack_allocator<int, 18> > >(a, an);
#endif
//#if __cplusplus >= 201103L
#ifdef LIBCPP_TEST_MIN_ALLOCATOR
    test<vector<int, min_allocator<int>> >(input_iterator<const int*>(a), input_iterator<const int*>(an));
    test<vector<int, min_allocator<int>> >(forward_iterator<const int*>(a), forward_iterator<const int*>(an));
    test<vector<int, min_allocator<int>> >(bidirectional_iterator<const int*>(a), bidirectional_iterator<const int*>(an));
    test<vector<int, min_allocator<int>> >(random_access_iterator<const int*>(a), random_access_iterator<const int*>(an));
    test<vector<int> >(a, an);
#endif
}

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

// void swap(unordered_set& x, unordered_set& y);

//#include <unordered_set>
//#include <cassert>

void main()
{
#ifdef LIBCXX_TEST_FAILURE
    {
        int a1[] = {1, 3, 7, 9, 10};
        int a2[] = {0, 2, 4, 5, 6, 8, 11};
        unordered_set<int> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
        unordered_set<int> c2(a2, a2+sizeof(a2)/sizeof(a2[0]));
        unordered_set<int>::iterator i1 = c1.begin();
        unordered_set<int>::iterator i2 = c2.begin();
        swap(c1, c2);
        c1.erase(i2);
        c2.erase(i1);
        //unordered_set<int>::iterator j = i1;
        LIBCPP_CATCH(c1.erase(i1));
        //assert(false);
    }
#endif
}

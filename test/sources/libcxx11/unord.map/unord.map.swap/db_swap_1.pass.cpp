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

// template <class Key, class Value, class Hash = hash<Key>, class Pred = equal_to<Key>,
//           class Alloc = allocator<pair<const Key, Value>>>
// class unordered_map

// void swap(unordered_map& x, unordered_map& y);

//#include <unordered_map>
//#include <cassert>

void main()
{
#ifdef LIBCXX_TEST_FAILURE
    {
        typedef std::pair<int, int> P;
        P a1[] = {P(1, 1), P(3, 3), P(7, 7), P(9, 9), P(10, 10)};
        P a2[] = {P(0, 0), P(2, 2), P(4, 4), P(5, 5), P(6, 6), P(8, 8), P(11, 11)};
        unordered_map<int, int> c1(a1, a1+sizeof(a1)/sizeof(a1[0]));
        unordered_map<int, int> c2(a2, a2+sizeof(a2)/sizeof(a2[0]));
        unordered_map<int, int>::iterator i1 = c1.begin();
        unordered_map<int, int>::iterator i2 = c2.begin();
        swap(c1, c2);
        c1.erase(i2);
        c2.erase(i1);
        //unordered_map<int, int>::iterator j = i1;
        LIBCPP_CATCH(c1.erase(i1));
        //assert(false);
    }
#endif
}

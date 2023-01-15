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

// <vector>

// class vector

// allocator_type get_allocator() const

//#include <vector>
//#include <cassert>

//#include "test_allocator.h"
//#include "test_macros.h"

TEST_CONSTEXPR_CXX20 bool test() {
    {
        std::allocator<int> alloc;
        const vector<int> v(alloc);
        assert(v.get_allocator() == alloc);
    }
    {
        other_allocator<int> alloc(1);
        const vector<int, other_allocator<int> > v(alloc);
        assert(v.get_allocator() == alloc);
    }

    return true;
}

void main() {
    test();
//#if TEST_STD_VER > 17
//    static_assert(test());
//#endif
}

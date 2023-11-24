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

// <set>

// class set

// set(const allocator_type& a);

void main()
{
    typedef std::less<int> C;
    typedef test_allocator<int> A;
    std::set<int, C, A> m(A(5));
    assert(m.empty());
    assert(m.begin() == m.end());
    assert(m.get_allocator() == A(5));
}

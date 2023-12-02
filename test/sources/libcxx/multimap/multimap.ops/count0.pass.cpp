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

//
// XFAIL: c++03, c++11

// <map>

// class multimap

// size_type count(const key_type& k) const;
//
//   The member function templates find, count, lower_bound, upper_bound, and
// equal_range shall not participate in overload resolution unless the
// qualified-id Compare::is_transparent is valid and denotes a type


int main(int, char**)
{
    {
    typedef std::multimap<int, double, transparent_less> M;
    assert(M().count(C2Int{5}) == 0);
    }
    {
    typedef std::multimap<int, double, transparent_less_not_referenceable> M;
    assert(M().count(C2Int{5}) == 0);
    }

  return 0;
}

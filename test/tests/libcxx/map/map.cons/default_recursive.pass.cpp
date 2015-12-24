//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

// <map>

// class map

// map();

//#include <map>

struct X
{
    map<int, X> m;
    map<int, X>::iterator i;
    map<int, X>::const_iterator ci;
    map<int, X>::reverse_iterator ri;
    map<int, X>::const_reverse_iterator cri;
};

void main()
{
}

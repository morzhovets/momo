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

#ifndef __PRIVATE_CONSTRUCTOR__H
#define __PRIVATE_CONSTRUCTOR__H

#include <iostream>

struct PrivateConstructor {

    PrivateConstructor static make ( int v ) { return PrivateConstructor(v); }
    int get () const { return val; }
private:
    PrivateConstructor ( int v ) : val(v) {}
    int val;
    };

inline bool operator < ( const PrivateConstructor &lhs, const PrivateConstructor &rhs ) { return lhs.get() < rhs.get(); }

inline bool operator < ( const PrivateConstructor &lhs, int rhs ) { return lhs.get() < rhs; }
inline bool operator < ( int lhs, const PrivateConstructor &rhs ) { return lhs < rhs.get(); }

inline std::ostream & operator << ( std::ostream &os, const PrivateConstructor &foo ) { return os << foo.get (); }

#endif

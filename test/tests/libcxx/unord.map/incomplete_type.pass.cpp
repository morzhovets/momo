
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

// Check that unordered_map and its iterators can be instantiated with an incomplete
// type.

//#include <unordered_map>

template <class Tp>
struct MyHash {
  MyHash() {}
  std::size_t operator()(Tp const&) const {return 42;}
};

struct A {
    typedef unordered_map<A, A, MyHash<A> > Map;
    Map m;
    Map::iterator it;
    Map::const_iterator cit;
    Map::local_iterator lit;
    Map::const_local_iterator clit;
};

inline bool operator==(A const& L, A const& R) { return &L == &R; }

void main() {
    A a;
}

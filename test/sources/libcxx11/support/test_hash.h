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

#ifndef TEST_HASH_H
#define TEST_HASH_H

#include <cstddef>
#include <type_traits>

template <class C>
class test_hash
    : private C
{
    int data_;
public:
    explicit test_hash(int data = 0) : data_(data) {}

    template<typename T>
    std::size_t operator()(const T& x) const
    //operator()(typename std::add_lvalue_reference<const typename C::argument_type>::type x) const
        {return C::operator()(x);}

    bool operator==(const test_hash& c) const
        {return data_ == c.data_;}
};

#endif  // TEST_HASH_H

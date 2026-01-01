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

#ifndef CHECK_CONSECUTIVE_H
#define CHECK_CONSECUTIVE_H

// <unordered_multimap>

#include <cassert>
#include <stddef.h>

// Check consecutive equal keys in an unordered_multimap iterator
template <typename Iter, typename Key, typename Multiset>
void CheckConsecutiveKeys(Iter pos, Iter end, const Key& key, Multiset& values)
{
    while (!values.empty())
    {
        assert(pos != end);
        assert(pos->first == key);
        assert(values.find(pos->second) != values.end());
        values.erase(values.find(pos->second));
        ++pos;
    }
    assert(pos == end || pos->first != key);
}

#endif

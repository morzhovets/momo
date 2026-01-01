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

// <map>

// class map

// allocator_type get_allocator() const

int main(int, char**) {
    typedef std::pair<const int, std::string> ValueType;
    {
        std::allocator<ValueType> alloc;
        const std::map<int, std::string> m(alloc);
        assert(m.get_allocator() == alloc);
    }
    {
        other_allocator<ValueType> alloc(1);
        const std::map<int, std::string, std::less<int>,
                       other_allocator<ValueType> > m(alloc);
        assert(m.get_allocator() == alloc);
    }

    return 0;
}

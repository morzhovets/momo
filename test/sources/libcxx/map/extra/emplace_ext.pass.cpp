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

// UNSUPPORTED: c++03

// <map>

// class map

// template <class... Args>
//   pair<iterator, bool> emplace(Args&&... args);
// template <class... Args>
//   iterator emplace_hint(const_iterator position, Args&&... args);

void main()
{
    {
        typedef std::map<int, double> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.emplace(M::value_type(2, 3.5));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 3.5);

        r = m.emplace(1, 3.5);
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == 3.5);

        r = m.emplace();
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 3);
        assert(m.begin()->first == 0);
        assert(m.begin()->second == 0.0);

        r = m.emplace(std::make_tuple(-1, 4.5));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 4);
        assert(m.begin()->first == -1);
        assert(m.begin()->second == 4.5);
    }
    {
        typedef std::map<std::string, double> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r = m.emplace(std::piecewise_construct, std::forward_as_tuple(2, 'a'),
                                                  std::forward_as_tuple(3.5));
        assert(r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == "aa");
        assert(m.begin()->second == 3.5);

        r = m.emplace(std::piecewise_construct, std::forward_as_tuple(2, 'a'),
                                                std::forward_as_tuple(2.5));
        assert(!r.second);
        assert(r.first == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == "aa");
        assert(m.begin()->second == 3.5);
    }
    {
        typedef std::map<int, double> M;
        typedef M::iterator R;
        M m;
        R r = m.emplace_hint(m.end(), M::value_type(2, 3.5));
        assert(r == m.begin());
        assert(m.size() == 1);
        assert(m.begin()->first == 2);
        assert(m.begin()->second == 3.5);

        r = m.emplace_hint(m.end(), 1, 3.5);
        assert(r == m.begin());
        assert(m.size() == 2);
        assert(m.begin()->first == 1);
        assert(m.begin()->second == 3.5);

        r = m.emplace_hint(m.end());
        assert(r == m.begin());
        assert(m.size() == 3);
        assert(m.begin()->first == 0);
        assert(m.begin()->second == 0.0);

        r = m.emplace_hint(m.end(), std::make_tuple(-1, 4.5));
        assert(r == m.begin());
        assert(m.size() == 4);
        assert(m.begin()->first == -1);
        assert(m.begin()->second == 4.5);
    }
}

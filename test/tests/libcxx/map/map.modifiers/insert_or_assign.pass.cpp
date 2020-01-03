//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: c++03, c++11, c++14

// <map>

// class map

// template <class M>
//  pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj);            // C++17
// template <class M>
//  pair<iterator, bool> insert_or_assign(key_type&& k, M&& obj);                 // C++17
// template <class M>
//  iterator insert_or_assign(const_iterator hint, const key_type& k, M&& obj);   // C++17
// template <class M>
//  iterator insert_or_assign(const_iterator hint, key_type&& k, M&& obj);        // C++17

//#include <__config>
//#include <map>
//#include <cassert>
//#include <tuple>

//#include <iostream>

void main()
{
#ifndef _LIBCPP_HAS_NO_RVALUE_REFERENCES
#ifndef _LIBCPP_HAS_NO_VARIADICS

    { // pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj);
        typedef map<int, Moveable> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r;
        for ( int i = 0; i < 20; i += 2 )
            m.emplace ( i, Moveable(i, static_cast<double>(i)));
        assert(m.size() == 10);

        for (int i=0; i < 20; i += 2)
        {
            Moveable mv(i+1, i+1);
            r = m.insert_or_assign(i, std::move(mv));
            assert(m.size() == 10);
            assert(!r.second);                    // was not inserted
            assert(mv.moved());                   // was moved from
            assert(r.first->first == i);          // key
            assert(r.first->second.get() == i+1); // value
        }

        Moveable mv1(5, 5.0);
        r = m.insert_or_assign(-1, std::move(mv1));
        assert(m.size() == 11);
        assert(r.second);                    // was inserted
        assert(mv1.moved());                 // was moved from
        assert(r.first->first        == -1); // key
        assert(r.first->second.get() == 5);  // value

        Moveable mv2(9, 9.0);
        r = m.insert_or_assign(3, std::move(mv2));
        assert(m.size() == 12);
        assert(r.second);                   // was inserted
        assert(mv2.moved());                // was moved from
        assert(r.first->first        == 3); // key
        assert(r.first->second.get() == 9); // value

        Moveable mv3(-1, 5.0);
        r = m.insert_or_assign(117, std::move(mv3));
        assert(m.size() == 13);
        assert(r.second);                     // was inserted
        assert(mv3.moved());                  // was moved from
        assert(r.first->first        == 117); // key
        assert(r.first->second.get() == -1);  // value
    }
    { // pair<iterator, bool> insert_or_assign(key_type&& k, M&& obj);
        typedef map<Moveable, Moveable> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r;
        for ( int i = 0; i < 20; i += 2 )
            m.emplace ( Moveable(i, static_cast<double>(i)), Moveable(i+1, static_cast<double>(i+1)));
        assert(m.size() == 10);

        Moveable mvkey1(2, 2.0);
        Moveable mv1(4, 4.0);
        r = m.insert_or_assign(std::move(mvkey1), std::move(mv1));
        assert(m.size() == 10);
        assert(!r.second);                  // was not inserted
        assert(!mvkey1.moved());            // was not moved from
        assert(mv1.moved());                // was moved from
        assert(r.first->first == mvkey1);   // key
        assert(r.first->second.get() == 4); // value

        Moveable mvkey2(3, 3.0);
        Moveable mv2(5, 5.0);
        r = m.try_emplace(std::move(mvkey2), std::move(mv2));
        assert(m.size() == 11);
        assert(r.second);                   // was inserted
        assert(mv2.moved());                // was moved from
        assert(mvkey2.moved());             // was moved from
        assert(r.first->first.get()  == 3); // key
        assert(r.first->second.get() == 5); // value
    }
    { // iterator insert_or_assign(const_iterator hint, const key_type& k, M&& obj);
        typedef map<int, Moveable> M;
        M m;
        M::iterator r;
        for ( int i = 0; i < 20; i += 2 )
            m.emplace ( i, Moveable(i, static_cast<double>(i)));
        assert(m.size() == 10);
        M::const_iterator it = m.find(2);

        Moveable mv1(3, 3.0);
        int k2 = 2;
        r = m.insert_or_assign(it, k2, std::move(mv1));
        assert(m.size() == 10);
        assert(mv1.moved());           // was moved from
        assert(r->first        == k2); // key
        assert(r->second.get() == 3);  // value

        Moveable mv2(5, 5.0);
        int k3 = 3;
        r = m.insert_or_assign(it, k3, std::move(mv2));
        assert(m.size() == 11);
        assert(mv2.moved());           // was moved from
        assert(r->first        == k3); // key
        assert(r->second.get() == 5);  // value
    }
    { // iterator insert_or_assign(const_iterator hint, key_type&& k, M&& obj);
        typedef map<Moveable, Moveable> M;
        M m;
        M::iterator r;
        for ( int i = 0; i < 20; i += 2 )
            m.emplace ( Moveable(i, static_cast<double>(i)), Moveable(i+1, static_cast<double>(i+1)));
        assert(m.size() == 10);
        M::const_iterator it = std::next(m.cbegin());

        Moveable mvkey1(2, 2.0);
        Moveable mv1(4, 4.0);
        r = m.insert_or_assign(it, std::move(mvkey1), std::move(mv1));
        assert(m.size() == 10);
        assert(mv1.moved());          // was moved from
        assert(!mvkey1.moved());      // was not moved from
        assert(r->first == mvkey1);   // key
        assert(r->second.get() == 4); // value

        Moveable mvkey2(3, 3.0);
        Moveable mv2(5, 5.0);
        r = m.insert_or_assign(it, std::move(mvkey2), std::move(mv2));
        assert(m.size() == 11);
        assert(mv2.moved());           // was moved from
        assert(mvkey2.moved());        // was moved from
        assert(r->first.get()  == 3);  // key
        assert(r->second.get() == 5);  // value
    }

#endif  // _LIBCPP_HAS_NO_VARIADICS
#endif  // _LIBCPP_HAS_NO_RVALUE_REFERENCES
}

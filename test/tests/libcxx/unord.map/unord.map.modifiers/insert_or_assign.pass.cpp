//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: c++98, c++03, c++11, c++14

// <unordered_map>

// class unordered_map

// template <class M>
//  pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj);            // C++17
// template <class M>
//  pair<iterator, bool> insert_or_assign(key_type&& k, M&& obj);                 // C++17
// template <class M>
//  iterator insert_or_assign(const_iterator hint, const key_type& k, M&& obj);   // C++17
// template <class M>
//  iterator insert_or_assign(const_iterator hint, key_type&& k, M&& obj);        // C++17

//#include <unordered_map>
//#include <cassert>
//#include <tuple>

void main()
{

    { // pair<iterator, bool> insert_or_assign(const key_type& k, M&& obj);
        typedef unordered_map<int, Moveable> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r;
        for (int i = 0; i < 20; i += 2)
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
        typedef unordered_map<Moveable, Moveable> M;
        typedef std::pair<M::iterator, bool> R;
        M m;
        R r;
        for (int i = 0; i < 20; i += 2)
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
        typedef unordered_map<int, Moveable> M;
        M m;
        M::iterator r;
        for (int i = 0; i < 20; i += 2)
            m.emplace ( i, Moveable(i, static_cast<double>(i)));
        assert(m.size() == 10);
#ifndef MOMO_USE_UNORDERED_HINT_ITERATORS
        M::const_iterator it = m.find(2);

        Moveable mv1(3, 3.0);
        r = m.insert_or_assign(it, 2, std::move(mv1));
        assert(m.size() == 10);
        assert(mv1.moved());           // was moved from
        assert(r->first        == 2);  // key
        assert(r->second.get() == 3);  // value
#endif

        Moveable mv2(5, 5.0);
        r = m.insert_or_assign(/*it*/m.find(3), 3, std::move(mv2));
        assert(m.size() == 11);
        assert(mv2.moved());           // was moved from
        assert(r->first        == 3);  // key
        assert(r->second.get() == 5);  // value
    }
    { // iterator insert_or_assign(const_iterator hint, key_type&& k, M&& obj);
        typedef unordered_map<Moveable, Moveable> M;
        M m;
        M::iterator r;
        for (int i = 0; i < 20; i += 2)
            m.emplace ( Moveable(i, static_cast<double>(i)), Moveable(i+1, static_cast<double>(i+1)));
        assert(m.size() == 10);
#ifndef MOMO_USE_UNORDERED_HINT_ITERATORS
        M::const_iterator it = std::next(m.cbegin());

        Moveable mvkey1(2, 2.0);
        Moveable mv1(4, 4.0);
        r = m.insert_or_assign(it, std::move(mvkey1), std::move(mv1));
        assert(m.size() == 10);
        assert(mv1.moved());          // was moved from
        assert(!mvkey1.moved());      // was not moved from
        assert(r->first == mvkey1);   // key
        assert(r->second.get() == 4); // value
#endif

        Moveable mvkey2(3, 3.0);
        Moveable mv2(5, 5.0);
        r = m.insert_or_assign(/*it*/m.find(mvkey2), std::move(mvkey2), std::move(mv2));
        assert(m.size() == 11);
        assert(mv2.moved());           // was moved from
        assert(mvkey2.moved());        // was moved from
        assert(r->first.get()  == 3);  // key
        assert(r->second.get() == 5);  // value
    }

}

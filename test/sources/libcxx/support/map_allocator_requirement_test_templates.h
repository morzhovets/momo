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

#ifndef MAP_ALLOCATOR_REQUIREMENT_TEST_TEMPLATES_H
#define MAP_ALLOCATOR_REQUIREMENT_TEST_TEMPLATES_H

// <map>
// <unordered_map>

// class map
// class unordered_map

// insert(...);
// emplace(...);
// emplace_hint(...);

// UNSUPPORTED: c++03

#include <cassert>
#include <iterator>

#include "test_macros.h"
#include "count_new.h"
#include "container_test_types.h"


template <class Container>
void testMapInsert()
{
  typedef typename Container::value_type ValueTp;
  typedef typename Container::key_type Key;
  typedef typename Container::mapped_type Mapped;
  ConstructController* cc = getConstructController();
  cc->reset();
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
  typename Container::allocator_type alloc;
#else
  ConstructController cc1, cc2;
  typename Container::allocator_type alloc(&cc1, &cc2);
#endif
  {
    // Testing C::insert(const value_type&)
    Container c(alloc);
    const ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<const ValueTp&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<const Mapped&>();
#endif
    assert(c.insert(v).second);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      const ValueTp v2(42, 1);
      assert(c.insert(v2).second == false);
    }
  }
  {
    // Testing C::insert(value_type&)
    Container c(alloc);
    ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<const ValueTp&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<Mapped&>();
#endif
    assert(c.insert(v).second);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      ValueTp v2(42, 1);
      assert(c.insert(v2).second == false);
    }
  }
  {
    // Testing C::insert(value_type&&)
    Container c(alloc);
    ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp&&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<Mapped&&>();
#endif
    assert(c.insert(std::move(v)).second);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      ValueTp v2(42, 1);
      assert(c.insert(std::move(v2)).second == false);
    }
  }
  {
    // Testing C::insert(const value_type&&)
    Container c(alloc);
    const ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<const ValueTp&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<const Mapped&&>();
#endif
    assert(c.insert(std::move(v)).second);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      const ValueTp v2(42, 1);
      assert(c.insert(std::move(v2)).second == false);
    }
  }
  {
    // Testing C::insert({key, value})
    Container c(alloc);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp&&>();
#else
    cc1.expect<Key&&>();
    cc2.expect<Mapped&&>();
#endif
    assert(c.insert({42, 1}).second);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      const ValueTp v2(42, 1);
      assert(c.insert(std::move(v2)).second == false);
    }
  }
  {
    // Testing C::insert(std::initializer_list<ValueTp>)
    Container c(alloc);
    std::initializer_list<ValueTp> il = { ValueTp(1, 1), ValueTp(2, 1) };
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp const&>(2);
#else
    cc1.expect<const Key&>(2);
    cc2.expect<const Mapped&>(2);
#endif
    c.insert(il);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      c.insert(il);
    }
  }
  {
    // Testing C::insert(Iter, Iter) for *Iter = value_type const&
    Container c(alloc);
    const ValueTp ValueList[] = { ValueTp(1, 1), ValueTp(2, 1), ValueTp(3, 1) };
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp const&>(3);
#else
    cc1.expect<const Key&>(3);
    cc2.expect<const Mapped&>(3);
#endif
    c.insert(std::begin(ValueList), std::end(ValueList));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      c.insert(std::begin(ValueList), std::end(ValueList));
    }
  }
  {
    // Testing C::insert(Iter, Iter) for *Iter = value_type&&
    Container c(alloc);
    ValueTp ValueList[] = { ValueTp(1, 1), ValueTp(2, 1) , ValueTp(3, 1) };
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp&&>(3);
#else
    cc1.expect<const Key&>(3);
    cc2.expect<Mapped&&>(3);
#endif
    c.insert(std::move_iterator<ValueTp*>(std::begin(ValueList)),
             std::move_iterator<ValueTp*>(std::end(ValueList)));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      ValueTp ValueList2[] = { ValueTp(1, 1), ValueTp(2, 1) , ValueTp(3, 1) };
      c.insert(std::move_iterator<ValueTp*>(std::begin(ValueList2)),
               std::move_iterator<ValueTp*>(std::end(ValueList2)));
    }
  }
  {
    // Testing C::insert(Iter, Iter) for *Iter = value_type&
    Container c(alloc);
    ValueTp ValueList[] = { ValueTp(1, 1), ValueTp(2, 1) , ValueTp(3, 1) };
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp const&>(3);
#else
    cc1.expect<const Key&>(3);
    cc2.expect<const Mapped&>(3);
#endif
    c.insert(std::begin(ValueList), std::end(ValueList));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      c.insert(std::begin(ValueList), std::end(ValueList));
    }
  }
}


template <class Container>
void testMapInsertHint()
{
  typedef typename Container::value_type ValueTp;
  typedef typename Container::key_type Key;
  typedef typename Container::mapped_type Mapped;
  typedef typename std::pair<Key, Mapped> NonConstKeyPair;
  typedef Container C;
  typedef typename C::iterator It;
  ConstructController* cc = getConstructController();
  cc->reset();
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
  typename Container::allocator_type alloc;
#else
  ConstructController cc1, cc2;
  typename Container::allocator_type alloc(&cc1, &cc2);
#endif
  {
    // Testing C::insert(p, const value_type&)
    Container c(alloc);
    const ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<const ValueTp&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<const Mapped&>();
#endif
    It ret = c.insert(c.end(), v);
    assert(ret != c.end());
    assert(c.size() == 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      const ValueTp v2(42, 1);
      It ret2 = c.insert(c.begin(), v2);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(&(*ret2) == &(*ret));
#else
      assert(&(*ret2).first == &(*ret).first);
#endif
      assert(c.size() == 1);
    }
  }
  {
    // Testing C::insert(p, value_type&)
    Container c(alloc);
    ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp const&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<Mapped&>();
#endif
    It ret = c.insert(c.end(), v);
    assert(ret != c.end());
    assert(c.size() == 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      ValueTp v2(42, 1);
      It ret2 = c.insert(c.begin(), v2);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(&(*ret2) == &(*ret));
#else
      assert(&(*ret2).first == &(*ret).first);
#endif
      assert(c.size() == 1);
    }
  }
  {
    // Testing C::insert(p, value_type&&)
    Container c(alloc);
    ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp&&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<Mapped&&>();
#endif
    It ret = c.insert(c.end(), std::move(v));
    assert(ret != c.end());
    assert(c.size() == 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      ValueTp v2(42, 1);
      It ret2 = c.insert(c.begin(), std::move(v2));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(&(*ret2) == &(*ret));
#else
      assert(&(*ret2).first == &(*ret).first);
#endif
      assert(c.size() == 1);
    }
  }
  {
    // Testing C::insert(p, {key, value})
    Container c(alloc);
    cc->expect<ValueTp&&>();
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp&&>();
#else
    cc1.expect<Key&&>();
    cc2.expect<Mapped&&>();
#endif
    It ret = c.insert(c.end(), {42, 1});
    assert(ret != c.end());
    assert(c.size() == 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      It ret2 = c.insert(c.begin(), {42, 1});
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(&(*ret2) == &(*ret));
#else
      assert(&(*ret2).first == &(*ret).first);
#endif
      assert(c.size() == 1);
    }
  }
  {
    // Testing C::insert(p, const value_type&&)
    Container c(alloc);
    const ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<const ValueTp&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<const Mapped&&>();
#endif
    It ret = c.insert(c.end(), std::move(v));
    assert(ret != c.end());
    assert(c.size() == 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      const ValueTp v2(42, 1);
      It ret2 = c.insert(c.begin(), std::move(v2));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(&(*ret2) == &(*ret));
#else
      assert(&(*ret2).first == &(*ret).first);
#endif
      assert(c.size() == 1);
    }
  }
  {
    // Testing C::insert(p, pair<Key, Mapped> const&)
    Container c(alloc);
    const NonConstKeyPair v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<const NonConstKeyPair&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<const Mapped&>();
#endif
    It ret = c.insert(c.end(), v);
    assert(ret != c.end());
    assert(c.size() == 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      const NonConstKeyPair v2(42, 1);
      It ret2 = c.insert(c.begin(), v2);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(&(*ret2) == &(*ret));
#else
      assert(&(*ret2).first == &(*ret).first);
#endif
      assert(c.size() == 1);
    }
  }
  {
    // Testing C::insert(p, pair<Key, Mapped>&&)
    Container c(alloc);
    NonConstKeyPair v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<NonConstKeyPair&&>();
#else
    cc1.expect<Key&&>();
    cc2.expect<Mapped&&>();
#endif
    It ret = c.insert(c.end(), std::move(v));
    assert(ret != c.end());
    assert(c.size() == 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      NonConstKeyPair v2(42, 1);
      It ret2 = c.insert(c.begin(), std::move(v2));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(&(*ret2) == &(*ret));
#else
      assert(&(*ret2).first == &(*ret).first);
#endif
      assert(c.size() == 1);
    }
  }


}


template <class Container>
void testMapEmplace()
{
  typedef typename Container::value_type ValueTp;
  typedef typename Container::key_type Key;
  typedef typename Container::mapped_type Mapped;
  typedef typename std::pair<Key, Mapped> NonConstKeyPair;
  ConstructController* cc = getConstructController();
  cc->reset();
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
  typename Container::allocator_type alloc;
#else
  ConstructController cc1, cc2;
  typename Container::allocator_type alloc(&cc1, &cc2);
#endif
  {
    // Testing C::emplace(const value_type&)
    Container c(alloc);
    const ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<const ValueTp&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<const Mapped&>();
#endif
    assert(c.emplace(v).second);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      const ValueTp v2(42, 1);
      assert(c.emplace(v2).second == false);
    }
  }
  {
    // Testing C::emplace(value_type&)
    Container c(alloc);
    ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<Mapped&>();
#endif
    assert(c.emplace(v).second);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      ValueTp v2(42, 1);
      assert(c.emplace(v2).second == false);
    }
  }
  {
    // Testing C::emplace(value_type&&)
    Container c(alloc);
    ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp&&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<Mapped&&>();
#endif
    assert(c.emplace(std::move(v)).second);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      ValueTp v2(42, 1);
      assert(c.emplace(std::move(v2)).second == false);
    }
  }
  {
    // Testing C::emplace(const value_type&&)
    Container c(alloc);
    const ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<const ValueTp&&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<const Mapped&&>();
#endif
    assert(c.emplace(std::move(v)).second);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      const ValueTp v2(42, 1);
      assert(c.emplace(std::move(v2)).second == false);
    }
  }
  {
    // Testing C::emplace(pair<Key, Mapped> const&)
    Container c(alloc);
    const NonConstKeyPair v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<const NonConstKeyPair&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<const Mapped&>();
#endif
    assert(c.emplace(v).second);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      const NonConstKeyPair v2(42, 1);
      assert(c.emplace(v2).second == false);
    }
  }
  {
    // Testing C::emplace(pair<Key, Mapped> &&)
    Container c(alloc);
    NonConstKeyPair v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<NonConstKeyPair&&>();
#else
    cc1.expect<Key&&>();
    cc2.expect<Mapped&&>();
#endif
    assert(c.emplace(std::move(v)).second);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      NonConstKeyPair v2(42, 1);
      assert(c.emplace(std::move(v2)).second == false);
    }
  }
  {
    // Testing C::emplace(const Key&, ConvertibleToMapped&&)
    Container c(alloc);
    const Key k(42);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<Key const&, int&&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<int&&>();
#endif
    assert(c.emplace(k, 1).second);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      const Key k2(42);
      assert(c.emplace(k2, 2).second == false);
    }
  }
  {
    // Testing C::emplace(Key&, Mapped&)
    Container c(alloc);
    Key k(42);
    Mapped m(1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<Key&, Mapped&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<Mapped&>();
#endif
    assert(c.emplace(k, m).second);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      Key k2(42);
      assert(c.emplace(k2, m).second == false);
    }
  }
  {
    // Testing C::emplace(Key&&, Mapped&&)
    Container c(alloc);
    Key k(42);
    Mapped m(1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<Key&&, Mapped&&>();
#else
    cc1.expect<Key&&>();
    cc2.expect<Mapped&&>();
#endif
    assert(c.emplace(std::move(k), std::move(m)).second);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      Key k2(42);
      Mapped m2(2);
      assert(c.emplace(std::move(k2), std::move(m2)).second == false);
    }
  }
  {
    // Testing C::emplace(ConvertibleToKey&&, ConvertibleToMapped&&)
    Container c(alloc);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<int&&, int&&>();
#else
    cc1.expect<int&&>();
    cc2.expect<int&&>();
#endif
    assert(c.emplace(42, 1).second);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      // test that emplacing a duplicate item allocates. We cannot optimize
      // this case because int&& does not match the type of key exactly.
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      cc->expect<int&&, int&&>();
#else
      cc1.expect<int&&>();
#endif
      assert(c.emplace(42, 1).second == false);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(!cc->unchecked());
#else
      assert(!cc1.unchecked());
      assert(!cc2.unchecked());
#endif
    }
  }
}


template <class Container>
void testMapEmplaceHint()
{
  typedef typename Container::value_type ValueTp;
  typedef typename Container::key_type Key;
  typedef typename Container::mapped_type Mapped;
  typedef typename std::pair<Key, Mapped> NonConstKeyPair;
  typedef Container C;
  typedef typename C::iterator It;
  ConstructController* cc = getConstructController();
  cc->reset();
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
  typename Container::allocator_type alloc;
#else
  ConstructController cc1, cc2;
  typename Container::allocator_type alloc(&cc1, &cc2);
#endif
  {
    // Testing C::emplace_hint(p, const value_type&)
    Container c(alloc);
    const ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<const ValueTp&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<const Mapped&>();
#endif
    It ret = c.emplace_hint(c.end(), v);
    assert(ret != c.end());
    assert(c.size() == 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      const ValueTp v2(42, 1);
      It ret2 = c.emplace_hint(c.begin(), v2);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(&(*ret2) == &(*ret));
#else
      assert(&(*ret2).first == &(*ret).first);
#endif
      assert(c.size() == 1);
    }
  }
  {
    // Testing C::emplace_hint(p, value_type&)
    Container c(alloc);
    ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<Mapped&>();
#endif
    It ret = c.emplace_hint(c.end(), v);
    assert(ret != c.end());
    assert(c.size() == 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      ValueTp v2(42, 1);
      It ret2 = c.emplace_hint(c.begin(), v2);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(&(*ret2) == &(*ret));
#else
      assert(&(*ret2).first == &(*ret).first);
#endif
      assert(c.size() == 1);
    }
  }
  {
    // Testing C::emplace_hint(p, value_type&&)
    Container c(alloc);
    ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp&&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<Mapped&&>();
#endif
    It ret = c.emplace_hint(c.end(), std::move(v));
    assert(ret != c.end());
    assert(c.size() == 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      ValueTp v2(42, 1);
      It ret2 = c.emplace_hint(c.begin(), std::move(v2));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(&(*ret2) == &(*ret));
#else
      assert(&(*ret2).first == &(*ret).first);
#endif
      assert(c.size() == 1);
    }
  }
  {
    // Testing C::emplace_hint(p, const value_type&&)
    Container c(alloc);
    const ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<const ValueTp&&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<const Mapped&&>();
#endif
    It ret = c.emplace_hint(c.end(), std::move(v));
    assert(ret != c.end());
    assert(c.size() == 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      const ValueTp v2(42, 1);
      It ret2 = c.emplace_hint(c.begin(), std::move(v2));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(&(*ret2) == &(*ret));
#else
      assert(&(*ret2).first == &(*ret).first);
#endif
      assert(c.size() == 1);
    }
  }
  {
    // Testing C::emplace_hint(p, pair<Key, Mapped> const&)
    Container c(alloc);
    const NonConstKeyPair v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<const NonConstKeyPair&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<const Mapped&>();
#endif
    It ret = c.emplace_hint(c.end(), v);
    assert(ret != c.end());
    assert(c.size() == 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      const NonConstKeyPair v2(42, 1);
      It ret2 = c.emplace_hint(c.begin(), v2);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(&(*ret2) == &(*ret));
#else
      assert(&(*ret2).first == &(*ret).first);
#endif
      assert(c.size() == 1);
    }
  }
  {
    // Testing C::emplace_hint(p, pair<Key, Mapped>&&)
    Container c(alloc);
    NonConstKeyPair v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<NonConstKeyPair&&>();
#else
    cc1.expect<Key&&>();
    cc2.expect<Mapped&&>();
#endif
    It ret = c.emplace_hint(c.end(), std::move(v));
    assert(ret != c.end());
    assert(c.size() == 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      NonConstKeyPair v2(42, 1);
      It ret2 = c.emplace_hint(c.begin(), std::move(v2));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(&(*ret2) == &(*ret));
#else
      assert(&(*ret2).first == &(*ret).first);
#endif
      assert(c.size() == 1);
    }
  }
  {
    // Testing C::emplace_hint(p, const Key&, ConvertibleToMapped&&)
    Container c(alloc);
    const Key k(42);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<Key const&, int&&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<int&&>();
#endif
    It ret = c.emplace_hint(c.end(), k, 42);
    assert(ret != c.end());
    assert(c.size() == 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      const Key k2(42);
      It ret2 = c.emplace_hint(c.begin(), k2, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(&(*ret2) == &(*ret));
#else
      assert(&(*ret2).first == &(*ret).first);
#endif
      assert(c.size() == 1);
    }
  }
  {
    // Testing C::emplace_hint(p, Key&, Mapped&)
    Container c(alloc);
    Key k(42);
    Mapped m(1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<Key&, Mapped&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<Mapped&>();
#endif
    It ret = c.emplace_hint(c.end(), k, m);
    assert(ret != c.end());
    assert(c.size() == 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      Key k2(42);
      Mapped m2(2);
      It ret2 = c.emplace_hint(c.begin(), k2, m2);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(&(*ret2) == &(*ret));
#else
      assert(&(*ret2).first == &(*ret).first);
#endif
      assert(c.size() == 1);
    }
  }
  {
    // Testing C::emplace_hint(p, Key&&, Mapped&&)
    Container c(alloc);
    Key k(42);
    Mapped m(1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<Key&&, Mapped&&>();
#else
    cc1.expect<Key&&>();
    cc2.expect<Mapped&&>();
#endif
    It ret = c.emplace_hint(c.end(), std::move(k), std::move(m));
    assert(ret != c.end());
    assert(c.size() == 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
      DisableAllocationGuard g;
      Key k2(42);
      Mapped m2(2);
      It ret2 = c.emplace_hint(c.begin(), std::move(k2), std::move(m2));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(&(*ret2) == &(*ret));
#else
      assert(&(*ret2).first == &(*ret).first);
#endif
      assert(c.size() == 1);
    }
  }
  {
    // Testing C::emplace_hint(p, ConvertibleToKey&&, ConvertibleToMapped&&)
    Container c(alloc);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<int&&, int&&>();
#else
    cc1.expect<int&&>();
    cc2.expect<int&&>();
#endif
    It ret = c.emplace_hint(c.end(), 42, 1);
    assert(ret != c.end());
    assert(c.size() == 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
    {
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      cc->expect<int&&, int&&>();
#else
      cc1.expect<int&&>();
#endif
      It ret2 = c.emplace_hint(c.begin(), 42, 2);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(&(*ret2) == &(*ret));
#else
      assert(&(*ret2).first == &(*ret).first);
#endif
      assert(c.size() == 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
      assert(!cc->unchecked());
#else
      assert(!cc1.unchecked());
      assert(!cc2.unchecked());
#endif
    }
  }

}


template <class Container>
void testMultimapInsert()
{
  typedef typename Container::value_type ValueTp;
  typedef typename Container::key_type Key;
  typedef typename Container::mapped_type Mapped;
  ConstructController* cc = getConstructController();
  cc->reset();
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
  typename Container::allocator_type alloc;
#else
  ConstructController cc1, cc2;
  typename Container::allocator_type alloc(&cc1, &cc2);
#endif
  {
    // Testing C::insert(const value_type&)
    Container c(alloc);
    const ValueTp v(42, 1);
    cc->expect<const ValueTp&>();
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<const ValueTp&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<const Mapped&>();
#endif
    c.insert(v);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
  }
  {
    // Testing C::insert(value_type&)
    Container c(alloc);
    ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<Mapped&>();
#endif
    c.insert(v);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
  }
  {
    // Testing C::insert(value_type&&)
    Container c(alloc);
    ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp&&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<Mapped&&>();
#endif
    c.insert(std::move(v));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
  }
  {
    // Testing C::insert({key, value})
    Container c(alloc);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp&&>();
#else
    cc1.expect<Key&&>();
    cc2.expect<Mapped&&>();
#endif
    c.insert({42, 1});
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
  }
  {
    // Testing C::insert(std::initializer_list<ValueTp>)
    Container c(alloc);
    std::initializer_list<ValueTp> il = { ValueTp(1, 1), ValueTp(2, 1) };
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp const&>(2);
#else
    cc1.expect<const Key&>(2);
    cc2.expect<const Mapped&>(2);
#endif
    c.insert(il);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
  }
  {
    // Testing C::insert(Iter, Iter) for *Iter = value_type const&
    Container c(alloc);
    const ValueTp ValueList[] = { ValueTp(1, 1), ValueTp(2, 1), ValueTp(3, 1) };
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp const&>(3);
#else
    cc1.expect<const Key&>(3);
    cc2.expect<const Mapped&>(3);
#endif
    c.insert(std::begin(ValueList), std::end(ValueList));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
  }
  {
    // Testing C::insert(Iter, Iter) for *Iter = value_type&&
    Container c(alloc);
    ValueTp ValueList[] = { ValueTp(1, 1), ValueTp(2, 1) , ValueTp(3, 1) };
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp&&>(3);
#else
    cc1.expect<const Key&>(3);
    cc2.expect<Mapped&&>(3);
#endif
    c.insert(std::move_iterator<ValueTp*>(std::begin(ValueList)),
             std::move_iterator<ValueTp*>(std::end(ValueList)));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
  }
  {
    // Testing C::insert(Iter, Iter) for *Iter = value_type&
    Container c(alloc);
    ValueTp ValueList[] = { ValueTp(1, 1), ValueTp(2, 1) , ValueTp(3, 1) };
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp&>(3);
#else
    cc1.expect<const Key&>(3);
    cc2.expect<const Mapped&>(3);
#endif
    c.insert(std::begin(ValueList), std::end(ValueList));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
  }
}


template <class Container>
void testMultimapInsertHint()
{
  typedef typename Container::value_type ValueTp;
  typedef typename Container::key_type Key;
  typedef typename Container::mapped_type Mapped;
  ConstructController* cc = getConstructController();
  cc->reset();
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
  typename Container::allocator_type alloc;
#else
  ConstructController cc1, cc2;
  typename Container::allocator_type alloc(&cc1, &cc2);
#endif
  {
    // Testing C::insert(p, const value_type&)
    Container c(alloc);
    const ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<const ValueTp&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<const Mapped&>();
#endif
    c.insert(c.begin(), v);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
  }
  {
    // Testing C::insert(p, value_type&)
    Container c(alloc);
    ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<Mapped&>();
#endif
    c.insert(c.begin(), v);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
  }
  {
    // Testing C::insert(p, value_type&&)
    Container c(alloc);
    ValueTp v(42, 1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp&&>();
#else
    cc1.expect<const Key&>();
    cc2.expect<Mapped&&>();
#endif
    c.insert(c.begin(), std::move(v));
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
  }
  {
    // Testing C::insert(p, {key, value})
    Container c(alloc);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    cc->expect<ValueTp&&>();
#else
    cc1.expect<Key&&>();
    cc2.expect<Mapped&&>();
#endif
    c.insert(c.begin(), {42, 1});
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    assert(!cc->unchecked());
#else
    assert(!cc1.unchecked());
    assert(!cc2.unchecked());
#endif
  }
}

#endif

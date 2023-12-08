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

// mapped_type& operator[](key_type&& k);

namespace TCT {
template <class Key = CopyInsertable<1>, class Value = CopyInsertable<2>,
          class ValueTp = std::pair<const Key, Value> >
using map =
      std::map<Key, Value, std::less<Key>,
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
                              ContainerTestAllocator<ValueTp, ValueTp>
#else
                              ContainerTestAllocatorForMap<ValueTp, Key, Value>
#endif
      >;
}

int main(int, char**)
{
    {
    std::map<MoveOnly, double> m;
    assert(m.size() == 0);
#ifndef MOMO_USE_SAFE_MAP_BRACKETS
    assert(m[1] == 0.0);
    assert(m.size() == 1);
#endif
    m[1] = -1.5;
    assert(m[1] == -1.5);
    assert(m.size() == 1);
#ifndef MOMO_USE_SAFE_MAP_BRACKETS
    assert(m[6] == 0);
    assert(m.size() == 2);
#endif
    m[6] = 6.5;
    assert(m[6] == 6.5);
    assert(m.size() == 2);
    }
    {
    typedef std::pair<const MoveOnly, double> V;
    std::map<MoveOnly, double, std::less<MoveOnly>, min_allocator<V>> m;
    assert(m.size() == 0);
#ifndef MOMO_USE_SAFE_MAP_BRACKETS
    assert(m[1] == 0.0);
    assert(m.size() == 1);
#endif
    m[1] = -1.5;
    assert(m[1] == -1.5);
    assert(m.size() == 1);
#ifndef MOMO_USE_SAFE_MAP_BRACKETS
    assert(m[6] == 0);
    assert(m.size() == 2);
#endif
    m[6] = 6.5;
    assert(m[6] == 6.5);
    assert(m.size() == 2);
    }
    {
        // Use "container_test_types.h" to check what arguments get passed
        // to the allocator for operator[]
        using Container = TCT::map<>;
        using Key = Container::key_type;
        using MappedType = Container::mapped_type;
        ConstructController* cc = getConstructController();
        cc->reset();
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
        typename Container::allocator_type alloc;
#else
        ConstructController cc1, cc2;
        typename Container::allocator_type alloc(&cc1, &cc2);
#endif
        {
            Container c(alloc);
            Key k(1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
            cc->expect<std::piecewise_construct_t const&, std::tuple<Key &&>&&, std::tuple<>&&>();
#else
            cc1.expect<Key&&>();
            cc2.expect<>();
#endif
            MappedType& mref = c[std::move(k)];
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
            assert(!cc->unchecked());
#else
            assert(!cc1.unchecked());
            assert(!cc2.unchecked());
#endif
            {
                Key k2(1);
                DisableAllocationGuard g;
                MappedType& mref2 = c[std::move(k2)];
                assert(&mref == &mref2);
            }
        }
    }

  return 0;
}

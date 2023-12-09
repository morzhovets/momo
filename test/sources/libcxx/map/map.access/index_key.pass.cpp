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

// mapped_type& operator[](const key_type& k);

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
    typedef std::pair<const int, double> V;
    V ar[] =
    {
        V(1, 1.5),
        V(2, 2.5),
        V(3, 3.5),
        V(4, 4.5),
        V(5, 5.5),
        V(7, 7.5),
        V(8, 8.5),
    };
    std::map<int, double> m(ar, ar+sizeof(ar)/sizeof(ar[0]));
    assert(m.size() == 7);
    assert(m[1] == 1.5);
    assert(m.size() == 7);
    m[1] = -1.5;
    assert(m[1] == -1.5);
    assert(m.size() == 7);
#ifndef MOMO_USE_SAFE_MAP_BRACKETS
    assert(m[6] == 0);
    assert(m.size() == 8);
#endif
    m[6] = 6.5;
    assert(m[6] == 6.5);
    assert(m.size() == 8);
    }
#if TEST_STD_VER >= 11
    {
    typedef std::pair<const int, double> V;
    V ar[] =
    {
        V(1, 1.5),
        V(2, 2.5),
        V(3, 3.5),
        V(4, 4.5),
        V(5, 5.5),
        V(7, 7.5),
        V(8, 8.5),
    };
    std::map<int, double, std::less<int>, min_allocator<V>> m(ar, ar+sizeof(ar)/sizeof(ar[0]));
    assert(m.size() == 7);
    assert(m[1] == 1.5);
    assert(m.size() == 7);
    const int i = 1;
    m[i] = -1.5;
    assert(m[1] == -1.5);
    assert(m.size() == 7);
#ifndef MOMO_USE_SAFE_MAP_BRACKETS
    assert(m[6] == 0);
    assert(m.size() == 8);
#endif
    m[6] = 6.5;
    assert(m[6] == 6.5);
    assert(m.size() == 8);
    }
#ifndef MOMO_USE_SAFE_MAP_BRACKETS
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
            const Key k(1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
            cc->expect<std::piecewise_construct_t const&, std::tuple<Key const&>&&, std::tuple<>&&>();
#else
            cc1.expect<const Key&>();
            cc2.expect<>();
#endif
            MappedType& mref = c[k];
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
            assert(!cc->unchecked());
#else
            assert(!cc1.unchecked());
            assert(!cc2.unchecked());
#endif
            {
                DisableAllocationGuard g;
                MappedType& mref2 = c[k];
                assert(&mref == &mref2);
            }
        }
        {
            Container c(alloc);
            Key k(1);
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
            cc->expect<std::piecewise_construct_t const&, std::tuple<Key const&>&&, std::tuple<>&&>();
#else
            cc1.expect<const Key&>();
            cc2.expect<>();
#endif
            MappedType& mref = c[k];
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
            assert(!cc->unchecked());
#else
            assert(!cc1.unchecked());
            assert(!cc2.unchecked());
#endif
            {
                DisableAllocationGuard g;
                MappedType& mref2 = c[k];
                assert(&mref == &mref2);
            }
        }
    }
#endif
#endif
#if TEST_STD_VER > 11
    {
    typedef std::pair<const int, double> V;
    V ar[] =
    {
        V(1, 1.5),
        V(2, 2.5),
        V(3, 3.5),
        V(4, 4.5),
        V(5, 5.5),
        V(7, 7.5),
        V(8, 8.5),
    };
    std::map<int, double, std::less<>> m(ar, ar+sizeof(ar)/sizeof(ar[0]));

    assert(m.size() == 7);
    assert(m[1] == 1.5);
    assert(m.size() == 7);
    m[1] = -1.5;
    assert(m[1] == -1.5);
    assert(m.size() == 7);
#ifndef MOMO_USE_SAFE_MAP_BRACKETS
    assert(m[6] == 0);
    assert(m.size() == 8);
#endif
    m[6] = 6.5;
    assert(m[6] == 6.5);
    assert(m.size() == 8);
    }
#endif

  return 0;
}

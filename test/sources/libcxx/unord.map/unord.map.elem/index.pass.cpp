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

// <unordered_map>

// template <class Key, class T, class Hash = hash<Key>, class Pred = equal_to<Key>,
//           class Alloc = allocator<pair<const Key, T>>>
// class unordered_map

// mapped_type& operator[](const key_type& k);
// mapped_type& operator[](key_type&& k);

namespace TCT {
template <class Key = CopyInsertable<1>, class Value = CopyInsertable<2>,
          class ValueTp = std::pair<const Key, Value> >
using unordered_map =
      std::unordered_map<Key, Value, std::hash<Key>, std::equal_to<Key>,
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
        typedef std::unordered_map<int, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a)/sizeof(a[0]));
        assert(c.size() == 4);
        c[1] = "ONE";
        assert(c.at(1) == "ONE");
        c[11] = "eleven";
        assert(c.size() == 5);
        assert(c.at(11) == "eleven");
    }
#if TEST_STD_VER >= 11
    {
        typedef std::unordered_map<MoveOnly, std::string> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a)/sizeof(a[0]));
        assert(c.size() == 4);
        c[1] = "ONE";
        assert(c.at(1) == "ONE");
        c[11] = "eleven";
        assert(c.size() == 5);
        assert(c.at(11) == "eleven");
    }
    {
        typedef std::unordered_map<int, std::string, std::hash<int>, std::equal_to<int>,
                            min_allocator<std::pair<const int, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a)/sizeof(a[0]));
        assert(c.size() == 4);
        c[1] = "ONE";
        assert(c.at(1) == "ONE");
        c[11] = "eleven";
        assert(c.size() == 5);
        assert(c.at(11) == "eleven");
    }

    {
        typedef std::unordered_map<MoveOnly, std::string, std::hash<MoveOnly>, std::equal_to<MoveOnly>,
                            min_allocator<std::pair<const MoveOnly, std::string>>> C;
        typedef std::pair<int, std::string> P;
        P a[] =
        {
            P(1, "one"),
            P(2, "two"),
            P(3, "three"),
            P(4, "four"),
            P(1, "four"),
            P(2, "four"),
        };
        C c(a, a + sizeof(a)/sizeof(a[0]));
        assert(c.size() == 4);
        c[1] = "ONE";
        assert(c.at(1) == "ONE");
        c[11] = "eleven";
        assert(c.size() == 5);
        assert(c.at(11) == "eleven");
    }
#ifndef MOMO_USE_SAFE_MAP_BRACKETS
    {
        using Container = TCT::unordered_map<>;
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
#endif
#endif

  return 0;
}

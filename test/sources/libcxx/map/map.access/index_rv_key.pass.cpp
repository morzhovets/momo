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
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    {
        // Use "container_test_types.h" to check what arguments get passed
        // to the allocator for operator[]
        using Container = TCT::map<>;
        using Key = Container::key_type;
        using MappedType = Container::mapped_type;
        ConstructController* cc = getConstructController();
        cc->reset();
        {
            Container c;
            Key k(1);
            cc->expect<std::piecewise_construct_t const&, std::tuple<Key &&>&&, std::tuple<>&&>();
            MappedType& mref = c[std::move(k)];
            assert(!cc->unchecked());
            {
                Key k2(1);
                DisableAllocationGuard g;
                MappedType& mref2 = c[std::move(k2)];
                assert(&mref == &mref2);
            }
        }
    }
#endif

  return 0;
}

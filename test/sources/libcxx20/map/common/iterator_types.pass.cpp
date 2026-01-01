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


template <class Map, class ValueTp, class PtrT, class CPtrT>
void testMap() {
  typedef typename Map::difference_type Diff;
  {
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    typedef typename Map::iterator It;
#else
    typedef std::iterator_traits<typename Map::iterator> It;
#endif
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    static_assert((std::is_same<typename It::value_type, ValueTp>::value), "");
    static_assert((std::is_same<typename It::reference, ValueTp&>::value), "");
    static_assert((std::is_same<typename It::pointer, PtrT>::value), "");
#endif
    static_assert((std::is_same<typename It::difference_type, Diff>::value), "");
  }
  {
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    typedef typename Map::const_iterator It;
#else
    typedef std::iterator_traits<typename Map::const_iterator> It;
#endif
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    static_assert((std::is_same<typename It::value_type, ValueTp>::value), "");
    static_assert((std::is_same<typename It::reference, ValueTp const&>::value), "");
    static_assert((std::is_same<typename It::pointer, CPtrT>::value), "");
#endif
    static_assert((std::is_same<typename It::difference_type, Diff>::value), "");
  }
}


int main(int, char**) {
  {
    typedef std::map<int, int> Map;
    typedef std::pair<const int, int> ValueTp;
    testMap<Map, ValueTp, ValueTp*, ValueTp const*>();
  }
  {
    typedef std::pair<const int, int> ValueTp;
    typedef test_allocator<ValueTp> Alloc;
    typedef std::map<int, int, std::less<int>, Alloc> Map;
    testMap<Map, ValueTp, ValueTp*, ValueTp const*>();
  }
#if TEST_STD_VER >= 11
  {
    typedef std::pair<const int, int> ValueTp;
    typedef min_allocator<ValueTp> Alloc;
    typedef std::map<int, int, std::less<int>, Alloc> Map;
    testMap<Map, ValueTp, min_pointer<ValueTp>, min_pointer<const ValueTp>>();
  }
#endif

  return 0;
}

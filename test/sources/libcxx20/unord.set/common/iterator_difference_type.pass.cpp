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


template <class Set, class ValueTp, class CPtrT>
void testUnorderedSet() {
  static_assert((std::is_convertible<typename Set::iterator,
                                     typename Set::const_iterator>::value), "");
  static_assert((std::is_convertible<typename Set::local_iterator,
                                     typename Set::const_local_iterator>::value), "");
  typedef typename Set::difference_type Diff;
  {
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    typedef typename Set::iterator It;
#else
    typedef std::iterator_traits<typename Set::iterator> It;
#endif
    static_assert((std::is_same<typename It::value_type, ValueTp>::value), "");
    static_assert((std::is_same<typename It::reference, ValueTp const&>::value), "");
    static_assert((std::is_same<typename It::pointer, CPtrT>::value), "");
    static_assert((std::is_same<typename It::difference_type, Diff>::value), "");

  }
  {
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
    typedef typename Set::iterator It;
#else
    typedef std::iterator_traits<typename Set::local_iterator> It;
#endif
    static_assert((std::is_same<typename It::value_type, ValueTp>::value), "");
    static_assert((std::is_same<typename It::reference, ValueTp const&>::value), "");
    static_assert((std::is_same<typename It::pointer, CPtrT>::value), "");
    static_assert((std::is_same<typename It::difference_type, Diff>::value), "");
  }
}

int main(int, char**) {
  {
    typedef int ValueTp;
    typedef std::unordered_set<ValueTp> Set;
    testUnorderedSet<Set, ValueTp, ValueTp const*>();
  }
  {
    typedef int ValueTp;
    typedef test_allocator<ValueTp> Alloc;
    typedef std::unordered_set<ValueTp, std::hash<ValueTp>, std::equal_to<ValueTp>, Alloc> Set;
    testUnorderedSet<Set, ValueTp, ValueTp const*>();
  }
#if TEST_STD_VER >= 11
  {
    typedef int ValueTp;
    typedef min_allocator<ValueTp> Alloc;
    typedef std::unordered_set<ValueTp, std::hash<ValueTp>, std::equal_to<ValueTp>, Alloc> Set;
    testUnorderedSet<Set, ValueTp, min_pointer<const ValueTp>>();
  }
#endif

  return 0;
}

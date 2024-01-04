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

// class unordered_multimap

// size_type max_size() const;

int main(int, char**)
{
  typedef std::pair<const int, int> KV;
#ifdef LIBCPP_HAS_BAD_NEWS_FOR_MOMO
  {
    typedef limited_allocator<KV, 10> A;
    typedef std::unordered_multimap<int, int, std::hash<int>,
                                    std::equal_to<int>, A>
        C;
    C c;
    assert(c.max_size() <= 10);
    LIBCPP_ASSERT(c.max_size() == 10);
  }
#endif
  {
    typedef limited_allocator<KV, static_cast<std::size_t>(-1)> A;
    typedef std::unordered_multimap<int, int, std::hash<int>,
                                    std::equal_to<int>, A>
        C;
    const C::size_type max_dist =
        static_cast<C::size_type>(std::numeric_limits<C::difference_type>::max());
    C c;
    assert(c.max_size() <= max_dist);
    LIBCPP_ASSERT(c.max_size() == max_dist);
    }
    {
      typedef std::unordered_multimap<char, int> C;
    const C::size_type max_dist =
        static_cast<C::size_type>(std::numeric_limits<C::difference_type>::max());
      C c;
      assert(c.max_size() <= max_dist);
      assert(c.max_size() <= alloc_max_size(c.get_allocator()));
    }

  return 0;
}

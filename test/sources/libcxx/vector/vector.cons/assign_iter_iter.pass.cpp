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

// <vector>

// void assign(size_type n, const_reference v);


TEST_CONSTEXPR_CXX20 bool test() {
#if TEST_STD_VER >= 11
  int arr1[] = {42};
  int arr2[] = {1, 101, 42};
  {
    using T = EmplaceConstructibleMoveableAndAssignable<int>;
    using It = forward_iterator<int*>;
    {
      std::vector<T> v;
      v.assign(It(arr1), It(std::end(arr1)));
      assert(v[0].value == 42);
    }
    {
      std::vector<T> v;
      v.assign(It(arr2), It(std::end(arr2)));
      assert(v[0].value == 1);
      assert(v[1].value == 101);
      assert(v[2].value == 42);
    }
  }
  {
    using T = EmplaceConstructibleMoveableAndAssignable<int>;
    using It = cpp17_input_iterator<int*>;
    {
      std::vector<T> v;
      v.assign(It(arr1), It(std::end(arr1)));
#ifndef LIBCXX_TEST_INTCAP_ARRAY
      assert(v[0].copied == 0);
#endif
      assert(v[0].value == 42);
    }
    {
      std::vector<T> v;
      v.assign(It(arr2), It(std::end(arr2)));
      //assert(v[0].copied == 0);
      assert(v[0].value == 1);
      //assert(v[1].copied == 0);
      assert(v[1].value == 101);
#ifndef LIBCXX_TEST_INTCAP_ARRAY
      assert(v[2].copied == 0);
#endif
      assert(v[2].value == 42);
    }
  }
#endif

  // Test with a number of elements in the source range that is greater than capacity
  {
    typedef forward_iterator<int*> It;

    std::vector<int> dst(10);

    std::size_t n = dst.capacity() * 2;
    std::vector<int> src(n);

#ifdef LIBCXX_TEST_SEGMENTED_ARRAY
    dst.assign(forward_iterator(src.begin()), forward_iterator(src.end()));
#else
    dst.assign(It(src.data()), It(src.data() + src.size()));
#endif
    assert(dst == src);
  }

  return true;
}

int main(int, char**) {
  test();
#if TEST_STD_VER > 17
  //static_assert(test());
#endif
  return 0;
}

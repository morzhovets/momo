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

// UNSUPPORTED: no-exceptions

// (bug report: https://llvm.org/PR58392)
// Check that vector constructors don't leak memory when an operation inside the constructor throws an exception

struct GlobalMemCounter
{
  inline static size_t size = 0;
};

template <class T>
struct AllocatorBase {
  using value_type      = T;
  using is_always_equal = std::false_type;

  AllocatorBase() = default;

  template <class U>
  AllocatorBase(const AllocatorBase<U>&) {}

  T* allocate(std::size_t n)
  {
    GlobalMemCounter::size += n * sizeof(T);
    return std::allocator<T>().allocate(n);
  }

  void deallocate(T* ptr, std::size_t n)
  {
    GlobalMemCounter::size -= n * sizeof(T);
    std::allocator<T>().deallocate(ptr, n);
  }

  template <class U>
  friend bool operator==(const AllocatorBase&, const AllocatorBase<U>&) { return true; }
};

template <class T>
struct AllocatorThrow : public AllocatorBase<T> {
  template <class U>
  AllocatorThrow(const AllocatorThrow<U>&) {}

  AllocatorThrow(bool should_throw = true) {
    if (should_throw)
      throw 0;
  }
};

struct ThrowingT {
  int* throw_after_n_ = nullptr;
  ThrowingT() { throw 0; }

  ThrowingT(int& throw_after_n) : throw_after_n_(&throw_after_n) {
    if (throw_after_n == 0)
      throw 0;
    --throw_after_n;
  }

  ThrowingT(const ThrowingT&) {
    if (throw_after_n_ == nullptr || *throw_after_n_ == 0)
      throw 1;
    --*throw_after_n_;
  }

  ThrowingT& operator=(const ThrowingT&) {
    if (throw_after_n_ == nullptr || *throw_after_n_ == 0)
      throw 1;
    --*throw_after_n_;
    return *this;
  }
};

template <class IterCat>
struct Iterator {
  using iterator_category = IterCat;
  using difference_type   = std::ptrdiff_t;
  using value_type        = int;
  using reference         = int&;
  using pointer           = int*;

  mutable int i_;
  Iterator(int i = 0) : i_(i) {}
  int& operator*() const {
    if (i_ == 1)
      throw 1;
    return i_;
  }

  friend bool operator==(const Iterator& lhs, const Iterator& rhs) { return lhs.i_ == rhs.i_; }

  friend bool operator!=(const Iterator& lhs, const Iterator& rhs) { return lhs.i_ != rhs.i_; }

  Iterator& operator++() {
    ++i_;
    return *this;
  }

  Iterator operator++(int) {
    auto tmp = *this;
    ++i_;
    return tmp;
  }
};

void check_new_delete_called() {
  assert(GlobalMemCounter::size == 0);
  //assert(globalMemCounter.new_called == globalMemCounter.delete_called);
  //assert(globalMemCounter.new_array_called == globalMemCounter.delete_array_called);
  //assert(globalMemCounter.aligned_new_called == globalMemCounter.aligned_delete_called);
  //assert(globalMemCounter.aligned_new_array_called == globalMemCounter.aligned_delete_array_called);
}

template<typename T>
using AllocThrowVec = std::vector<T, AllocatorThrow<T>>;

template<typename T>
using AllocBaseVec = std::vector<T, AllocatorBase<T>>;

int main(int, char**) {
  try { // vector()
    AllocThrowVec<int> vec;
  } catch (int) {
  }
  check_new_delete_called();

  try { // Throw in vector(size_type) from type
    AllocBaseVec<ThrowingT> get_alloc(1);
  } catch (int) {
  }
  check_new_delete_called();

#if TEST_STD_VER >= 14
  try { // Throw in vector(size_type, value_type) from type
    int throw_after = 1;
    ThrowingT v(throw_after);
    AllocBaseVec<ThrowingT> get_alloc(1, v);
  } catch (int) {
  }
  check_new_delete_called();

  try { // Throw in vector(size_type, const allocator_type&) from allocator
    AllocatorThrow<int> alloc(false);
    AllocThrowVec<int> get_alloc(0, alloc);
  } catch (int) {
  }
  check_new_delete_called();

  try { // Throw in vector(size_type, const allocator_type&) from the type
    AllocBaseVec<ThrowingT> vec(1, AllocatorBase<ThrowingT>());
  } catch (int) {
  }
  check_new_delete_called();
#endif  // TEST_STD_VER >= 14

  try { // Throw in vector(InputIterator, InputIterator) from input iterator
    AllocBaseVec<int> vec((Iterator<std::input_iterator_tag>()), Iterator<std::input_iterator_tag>(2));
  } catch (int) {
  }
  check_new_delete_called();

  try { // Throw in vector(InputIterator, InputIterator) from forward iterator
    AllocBaseVec<int> vec((Iterator<std::forward_iterator_tag>()), Iterator<std::forward_iterator_tag>(2));
  } catch (int) {
  }
  check_new_delete_called();

  try { // Throw in vector(InputIterator, InputIterator) from allocator
    int a[] = {1, 2};
    AllocThrowVec<int> vec(cpp17_input_iterator<int*>(a), cpp17_input_iterator<int*>(a + 2));
  } catch (int) {
  }
  check_new_delete_called();

  try { // Throw in vector(InputIterator, InputIterator, const allocator_type&) from input iterator
    AllocatorBase<int> alloc;
    AllocBaseVec<int> vec(Iterator<std::input_iterator_tag>(), Iterator<std::input_iterator_tag>(2), alloc);
  } catch (int) {
  }
  check_new_delete_called();

  try { // Throw in vector(InputIterator, InputIterator, const allocator_type&) from forward iterator
    AllocatorBase<int> alloc;
    AllocBaseVec<int> vec(Iterator<std::forward_iterator_tag>(), Iterator<std::forward_iterator_tag>(2), alloc);
  } catch (int) {
  }
  check_new_delete_called();

  try { // Throw in vector(InputIterator, InputIterator, const allocator_type&) from allocator
    int a[] = {1, 2};
    AllocatorThrow<int> alloc(false);
    AllocThrowVec<int> vec(cpp17_input_iterator<int*>(a), cpp17_input_iterator<int*>(a + 2), alloc);
  } catch (int) {
    // FIXME: never called.
  }
  check_new_delete_called();

  try { // Throw in vector(InputIterator, InputIterator, const allocator_type&) from allocator
    int a[] = {1, 2};
    AllocatorThrow<int> alloc(false);
    AllocThrowVec<int> vec(forward_iterator<int*>(a), forward_iterator<int*>(a + 2), alloc);
  } catch (int) {
    // FIXME: never called.
  }
  check_new_delete_called();

  try { // Throw in vector(const vector&) from type
    AllocBaseVec<ThrowingT> vec;
    int throw_after = 0;
    vec.emplace_back(throw_after);
    auto vec2 = vec;
  } catch (int) {
  }
  check_new_delete_called();

  try { // Throw in vector(const vector&, const allocator_type&) from type
    AllocBaseVec<ThrowingT> vec;
    int throw_after = 1;
    vec.emplace_back(throw_after);
    AllocBaseVec<ThrowingT> vec2(vec, AllocatorBase<int>());
  } catch (int) {
  }
  check_new_delete_called();

#ifndef LIBCXX_TEST_INTCAP_ARRAY
  try { // Throw in vector(vector&&, const allocator_type&) from type
    AllocThrowVec<ThrowingT> vec(AllocatorThrow<ThrowingT>(false));
    int throw_after = 1;
    vec.emplace_back(throw_after);
    AllocThrowVec<ThrowingT> vec2(std::move(vec), AllocatorThrow<ThrowingT>(false));
  } catch (int) {
  }
  check_new_delete_called();
#endif

#if TEST_STD_VER >= 11
  try { // Throw in vector(initializer_list<value_type>) from type
    int throw_after = 1;
    AllocBaseVec<ThrowingT> vec({ThrowingT(throw_after)});
  } catch (int) {
  }
  check_new_delete_called();

  try { // Throw in vector(initializer_list<value_type>, const allocator_type&) constructor from type
    int throw_after = 1;
    AllocBaseVec<ThrowingT> vec({ThrowingT(throw_after)}, AllocatorBase<ThrowingT>());
  } catch (int) {
  }
  check_new_delete_called();
#endif // TEST_STD_VER >= 11

  return 0;
}

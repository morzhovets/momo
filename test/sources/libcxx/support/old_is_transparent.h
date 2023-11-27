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

#ifndef OLD_TRANSPARENT_H
#define OLD_TRANSPARENT_H

#include "test_macros.h"

template <typename T>
struct StoredType;

template <typename T>
struct SearchedType;

struct hash_impl {
  template <typename T>
  constexpr std::size_t operator()(SearchedType<T> const& t) const {
    return static_cast<std::size_t>(t.get_value());
  }

  template <typename T>
  constexpr std::size_t operator()(StoredType<T> const& t) const {
    return static_cast<std::size_t>(t.get_value());
  }
};

struct non_transparent_hash : hash_impl {};

struct transparent_hash : hash_impl {
  using is_transparent = void;
};

struct transparent_hash_final final : transparent_hash {};

struct transparent_equal_final final : std::equal_to<> {};

template <typename T>
struct SearchedType {
  SearchedType(T value, int* counter) : value_(value), conversions_(counter) { }

  // Whenever a conversion is performed, increment the counter to keep track
  // of conversions.
  operator StoredType<T>() const {
    ++*conversions_;
    return StoredType<T>{value_};
  }

  int get_value() const {
    return value_;
  }

private:
  T value_;
  int* conversions_;
};

template <typename T>
struct StoredType {
  StoredType() = default;
  StoredType(T value) : value_(value) { }

  friend bool operator==(StoredType const& lhs, StoredType const& rhs) {
    return lhs.value_ == rhs.value_;
  }

  // If we're being passed a SearchedType<T> object, avoid the conversion
  // to T. This allows testing that the transparent operations are correctly
  // forwarding the SearchedType all the way to this comparison by checking
  // that we didn't have a conversion when we search for a SearchedType<T>
  // in a container full of StoredType<T>.
  friend bool operator==(StoredType const& lhs, SearchedType<T> const& rhs) {
    return lhs.value_ == rhs.get_value();
  }
  friend bool operator==(SearchedType<T> const& lhs, StoredType<T> const& rhs) {
    return lhs.get_value() == rhs.value_;
  }

  int get_value() const {
    return value_;
  }

private:
  T value_;
};

#endif // OLD_TRANSPARENT_H

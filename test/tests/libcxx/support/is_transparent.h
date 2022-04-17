//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef TRANSPARENT_H
#define TRANSPARENT_H

// testing transparent
#ifndef LIBCPP_HAS_NO_TRANSPARENT_OPERATORS

struct transparent_less
{
    template <class T, class U>
    constexpr auto operator()(T&& t, U&& u) const
    noexcept(noexcept(std::forward<T>(t) < std::forward<U>(u)))
    -> decltype      (std::forward<T>(t) < std::forward<U>(u))
        { return      std::forward<T>(t) < std::forward<U>(u); }
    typedef void is_transparent;  // correct
};

struct transparent_less_no_type
{
    template <class T, class U>
    constexpr auto operator()(T&& t, U&& u) const
    noexcept(noexcept(std::forward<T>(t) < std::forward<U>(u)))
    -> decltype      (std::forward<T>(t) < std::forward<U>(u))
        { return      std::forward<T>(t) < std::forward<U>(u); }
private:
//    typedef void is_transparent;  // error - should exist
};

struct transparent_less_private
{
    template <class T, class U>
    constexpr auto operator()(T&& t, U&& u) const
    noexcept(noexcept(std::forward<T>(t) < std::forward<U>(u)))
    -> decltype      (std::forward<T>(t) < std::forward<U>(u))
        { return      std::forward<T>(t) < std::forward<U>(u); }
private:
    typedef void is_transparent;  // error - should be accessible
};

struct transparent_less_not_a_type
{
    template <class T, class U>
    constexpr auto operator()(T&& t, U&& u) const
    noexcept(noexcept(std::forward<T>(t) < std::forward<U>(u)))
    -> decltype      (std::forward<T>(t) < std::forward<U>(u))
        { return      std::forward<T>(t) < std::forward<U>(u); }

    int is_transparent;  // error - should be a type
};

struct C2Int { // comparable to int
    C2Int() : i_(0) {}
    C2Int(int i): i_(i) {}
    int get () const { return i_; }
private:
    int i_;
    };

inline bool operator <(int          rhs,   const C2Int& lhs) { return rhs       < lhs.get(); }
inline bool operator <(const C2Int& rhs,   const C2Int& lhs) { return rhs.get() < lhs.get(); }
inline bool operator <(const C2Int& rhs,            int lhs) { return rhs.get() < lhs; }


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

#endif

#endif  // TRANSPARENT_H

/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/UserSettings.h

  This file can be modified by user.

\**********************************************************/

#pragma once

#ifdef __has_include
#if __has_include(<version>)
#include <version>	// feature macros
#endif
#endif

// If you activate safe map brackets, in the case of absence in `map` the key `key`
// the expression `map[key]` can be used only on the left side of an assignment operator.
// Do not forget that the references to the items may become invalid after each insertion,
// so expressions such as `map[key1] + map[key2]` are potentially dangerous.
//#define MOMO_USE_SAFE_MAP_BRACKETS

// Using hint iterators in classes `stdish::unordered_set/map`.
// As a hint pass an iterator, returned after an unsuccessful search.
//#define MOMO_USE_UNORDERED_HINT_ITERATORS

// If your program does not use exceptions, define it as `true`.
// On the contrary, for strong safety it can be defined as `false`.
#define MOMO_IS_NOTHROW_RELOCATABLE_APPENDIX(Object) (!std::is_copy_constructible<Object>::value)

// Using `memcpy` for relocate
#define MOMO_IS_TRIVIALLY_RELOCATABLE(Object) (std::is_trivially_copyable<Object>::value)

#if defined(__GNUC__) && __GNUC__ < 5
#undef MOMO_IS_TRIVIALLY_RELOCATABLE
#define MOMO_IS_TRIVIALLY_RELOCATABLE(Object) (std::is_trivial<Object>::value)
#endif

//#define MOMO_MEM_MANAGER_PTR_USEFUL_BIT_COUNT ((sizeof(void*) == 8) ? 48 : sizeof(void*) * 8)

// If your platform does not require data alignment, define it as `1`
#define MOMO_MAX_ALIGNMENT alignof(std::max_align_t)

// Memory pool settings
#define MOMO_DEFAULT_MEM_POOL_BLOCK_COUNT 32
#define MOMO_DEFAULT_MEM_POOL_CACHED_FREE_BLOCK_COUNT 16

// The method of arguments check in functions (`assertion` or `exception`)
#define MOMO_DEFAULT_CHECK_MODE assertion

// The method of arguments extra check in functions (`assertion` or `nothing`)
#define MOMO_DEFAULT_EXTRA_CHECK_MODE assertion

// Checking iterators for invalidation
#ifdef NDEBUG
#define MOMO_CHECK_ITERATOR_VERSION (checkMode != CheckMode::assertion)
#else
#define MOMO_CHECK_ITERATOR_VERSION true
#endif

// Default bucket type in hash tables
#define MOMO_DEFAULT_HASH_BUCKET HashBucketLimP4<>

// Default bucket type in open addressing hash tables
#define MOMO_DEFAULT_HASH_BUCKET_OPEN HashBucketOpen8

// Settings of node in B-tree
#define MOMO_DEFAULT_TREE_NODE TreeNode<32, 4>

//#define MOMO_HASH_CODER(key) key.GetHashCode() //hash_value(key)

#ifdef __cpp_lib_string_view
#define MOMO_USE_HASH_TRAITS_STRING_SPECIALIZATION
#endif

// If hash function is slow, hash bucket can store part of hash code
// to avoid its recalculation during table grow
#define MOMO_IS_FAST_NOTHROW_HASHABLE(Key) (std::is_arithmetic<Key>::value)

// If key has fast `operator<`, linear search is used in the tree nodes instead of binary one
#define MOMO_IS_FAST_COMPARABLE(Key) (std::is_arithmetic<Key>::value || std::is_pointer<Key>::value)

#if defined(_MSC_VER) //defined(_WIN32)
#define MOMO_USE_MEM_MANAGER_WIN
#define MOMO_DEFAULT_MEM_MANAGER MemManagerWin
#elif defined(__linux__)
// Linux has fast `realloc`
#define MOMO_DEFAULT_MEM_MANAGER MemManagerC
#else
// Function `realloc` operates slowly under Windows and therefore is not used by default
#define MOMO_DEFAULT_MEM_MANAGER MemManagerCpp
#endif

// Using of SSE2
#if defined(_MSC_VER) && !defined(__clang__)
#if defined(_M_AMD64) || defined(_M_X64)
#define MOMO_USE_SSE2
#elif _M_IX86_FP == 2
#define MOMO_USE_SSE2
#endif
#else
#ifdef __SSE2__
#define MOMO_USE_SSE2
#endif
#endif

#if defined(__GNUC__) || defined(__clang__)
#define MOMO_CTZ32(value) __builtin_ctz(value)
#endif

// `nullptr`, converted to the type `uintptr_t`
#define MOMO_NULL_UINTPTR reinterpret_cast<uintptr_t>(static_cast<void*>(nullptr))

#if defined(__clang__)
#undef MOMO_NULL_UINTPTR
#define MOMO_NULL_UINTPTR uintptr_t{0}
#endif

// One more pointer which doesn't point to anything but is not equal to `nullptr`
#define MOMO_INVALID_UINTPTR (MOMO_NULL_UINTPTR + 1)

#define MOMO_ASSERT(expr) assert(expr)

#ifdef __cpp_deduction_guides
#define MOMO_HAS_DEDUCTION_GUIDES
#endif

#ifdef __cpp_guaranteed_copy_elision
#define MOMO_HAS_GUARANTEED_COPY_ELISION
#if defined(_MSC_VER) && !defined(__clang__)	// vs2017
#undef MOMO_HAS_GUARANTEED_COPY_ELISION
#endif
#endif

#ifdef __has_cpp_attribute
#if __has_cpp_attribute(nodiscard)
#define MOMO_NODISCARD [[nodiscard]]
#endif
#endif
#ifndef MOMO_NODISCARD
#define MOMO_NODISCARD
#endif

//#if defined(_MSC_VER) && !defined(__clang__)
//#pragma warning (disable: 4127)	// conditional expression is constant
//#pragma warning (disable: 4503)	// decorated name length exceeded, name was truncated
//#pragma warning (disable: 4510)	// default constructor could not be generated
//#pragma warning (disable: 4512)	// assignment operator could not be generated
//#define _SCL_SECURE_NO_WARNINGS
//#endif

/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/UserSettings.h

  This file can be modified by user.

\**********************************************************/

#pragma once

// If you activate safe map brackets, in the case of absence in `map` the key `key`
// the expression `map[key]` can be used only on the left side of an assignment operator.
// Do not forget that the references to the items may become invalid after each insertion,
// so expressions such as `map[key1] + map[key2]` are potentially dangerous.
//#define MOMO_USE_SAFE_MAP_BRACKETS

// Using hint iterators in classes `stdish::unordered_set/map/multimap`.
// As a hint pass an iterator, returned after an unsuccessful search.
//#define MOMO_USE_UNORDERED_HINT_ITERATORS

// If your program does not use exceptions, define it as `true`.
// On the contrary, for strong safety it can be defined as
// `std::is_nothrow_move_constructible<Object>::value`.
#define MOMO_IS_NOTHROW_MOVE_CONSTRUCTIBLE(Object) \
	(std::is_nothrow_move_constructible<Object>::value \
	|| (!std::is_copy_constructible<Object>::value && !momo::IsTriviallyRelocatable<Object>::value))

// Using `memcpy` for relocate
#define MOMO_IS_TRIVIALLY_RELOCATABLE(Object) (std::is_trivially_copyable<Object>::value)

#if defined(__GNUC__) && __GNUC__ < 5
#undef MOMO_IS_TRIVIALLY_RELOCATABLE
#define MOMO_IS_TRIVIALLY_RELOCATABLE(Object) (std::is_trivial<Object>::value)
#endif

// C++17: `std::is_nothrow_swappable_v<Object>`
#define MOMO_IS_NOTHROW_SWAPPABLE(Object) false

// If your platform does not require data alignment, define it as `1`
#define MOMO_MAX_ALIGNMENT (std::alignment_of<std::max_align_t>::value)

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
#define MOMO_DEFAULT_HASH_BUCKET HashBucketLimP<>

// Settings of node in B-tree
#define MOMO_DEFAULT_TREE_NODE TreeNode<32, 4>

// If key has fast `operator<`, linear search is used in the tree nodes instead of binary one
#define MOMO_IS_FAST_COMPARABLE(Key) (std::is_arithmetic<Key>::value || std::is_pointer<Key>::value)

#if defined(_MSC_VER) //defined(_WIN32)
// Function `realloc` operates slowly under Windows and therefore is not used by default
#define MOMO_USE_MEM_MANAGER_WIN
#define MOMO_DEFAULT_MEM_MANAGER MemManagerWin
#elif defined(__linux__)
// Linux has fast `realloc`
#define MOMO_DEFAULT_MEM_MANAGER MemManagerC
#else
#define MOMO_DEFAULT_MEM_MANAGER MemManagerCpp
#endif

// `nullptr`, converted to the type `uintptr_t`
#define MOMO_NULL_UINTPTR ((uintptr_t)(void*)nullptr)

#if defined(__clang__)
#undef MOMO_NULL_UINTPTR
#define MOMO_NULL_UINTPTR ((uintptr_t)0)
#endif

// One more pointer which doesn't point to anything but is not equal to `nullptr`
#define MOMO_INVALID_UINTPTR (MOMO_NULL_UINTPTR + 1)

#define MOMO_ASSERT(expr) assert(expr)

#define MOMO_NOEXCEPT noexcept
#define MOMO_NOEXCEPT_IF(expr) noexcept(expr)

#if defined(_MSC_VER) && _MSC_VER < 1900
#undef MOMO_NOEXCEPT
#undef MOMO_NOEXCEPT_IF
#define MOMO_NOEXCEPT throw()
#define MOMO_NOEXCEPT_IF(expr)
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4127)	// conditional expression is constant
#pragma warning (disable: 4503)	// decorated name length exceeded, name was truncated
#pragma warning (disable: 4510)	// default constructor could not be generated
#pragma warning (disable: 4512)	// assignment operator could not be generated
#pragma warning (disable: 4610)	// struct can never be instantiated - user defined constructor required
#define _SCL_SECURE_NO_WARNINGS
#endif

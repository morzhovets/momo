/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/UserSettings.h

  This file can be modified by user.

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_USER_SETTINGS
#define MOMO_INCLUDE_GUARD_USER_SETTINGS

#ifdef __has_include
# if __has_include(<version>)
#  include <version>	// feature macros
# endif
#endif

#include <cassert>

#if defined(__cpp_lib_bitops)
# include <bit>
#endif

// If you activate safe map brackets, in the case of absence in `map` the key `key`
// the expression `map[key]` can be used only on the left side of an assignment operator.
// Do not forget that the references to the items may become invalid after each insertion,
// so expressions such as `map[key1] + map[key2]` are potentially dangerous.
//#define MOMO_USE_SAFE_MAP_BRACKETS

// Using hint iterators in classes `stdish::unordered_set/map`.
// As a hint pass an iterator, returned after an unsuccessful search.
//#define MOMO_USE_UNORDERED_HINT_ITERATORS

// Disable use of `typeid` operator
//#define MOMO_DISABLE_TYPE_INFO

// This macro controls the move constructor usage when no exceptions should be thrown.
// By default, we can use the move constructor even if it is not marked as `noexcept`.
// If your program does not use exceptions at all, you can define it as `true`.
// On the contrary, for strong safety it can be defined as `false`.
#if defined(__GNUC__) || defined(__clang__)
// `false` if Object has copy constructor and no move constructor, works in GCC and Clang
# define MOMO_IS_NOTHROW_RELOCATABLE_APPENDIX(Object) \
	(!std::is_constructible<Object, momo::internal::ConvertibleToReferences<Object>>::value)
#else
// Logic from `std::vector`
# define MOMO_IS_NOTHROW_RELOCATABLE_APPENDIX(Object) (!std::is_copy_constructible<Object>::value)
#endif

// Using `memcpy` for relocate
#define MOMO_IS_TRIVIALLY_RELOCATABLE(Object) (std::is_trivially_copyable<Object>::value)
#if defined(__GNUC__) && !defined(__clang__) && __cplusplus < 202002L	// -Wclass-memaccess
# undef MOMO_IS_TRIVIALLY_RELOCATABLE
# define MOMO_IS_TRIVIALLY_RELOCATABLE(Object) \
	(std::is_trivially_copyable<Object>::value && std::is_trivially_copy_assignable<Object>::value)
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
# define MOMO_CHECK_ITERATOR_VERSION (checkMode != CheckMode::assertion)
#else
# define MOMO_CHECK_ITERATOR_VERSION true
#endif

// Default bucket type in hash tables
#define MOMO_DEFAULT_HASH_BUCKET HashBucketLimP4<>

// Default bucket type in open addressing hash tables
#define MOMO_DEFAULT_HASH_BUCKET_OPEN HashBucketOpen8

// Settings of node in B-tree
#define MOMO_DEFAULT_TREE_NODE TreeNode<>

//#define MOMO_HASH_CODER(key) hash_code(std::addressof(key))

#ifdef __cpp_lib_string_view
# define MOMO_USE_HASH_TRAITS_STRING_SPECIALIZATION
#endif

// If hash function is slow, hash bucket can store part of hash code
// to avoid its recalculation during table grow
#define MOMO_IS_FAST_NOTHROW_HASHABLE(Key) (std::is_arithmetic<Key>::value)

// If key has fast `operator<`, linear search is used in the tree nodes instead of binary one
#define MOMO_IS_FAST_COMPARABLE(Key) (std::is_arithmetic<Key>::value || std::is_pointer<Key>::value)

#define MOMO_DEFAULT_MEM_MANAGER MemManagerC
#define MOMO_USE_DEFAULT_MEM_MANAGER_IN_STD

// Inlining
#if defined(_MSC_VER)
# define MOMO_FORCEINLINE __forceinline
# define MOMO_NOINLINE __declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
# define MOMO_FORCEINLINE inline __attribute__((__always_inline__))
# define MOMO_NOINLINE __attribute__((__noinline__))
#else
# define MOMO_FORCEINLINE inline
# define MOMO_NOINLINE
#endif

// Using of SSE2
#if defined(_MSC_VER) && !defined(__clang__)
# if (defined(_M_X64) /*|| _M_IX86_FP == 2*/) && !defined(_M_CEE)
#  define MOMO_USE_SSE2
# endif
#else
# ifdef __SSE2__
#  define MOMO_USE_SSE2
# endif
#endif

#if defined(_WIN32) || (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) \
	&& __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
# define MOMO_LITTLE_ENDIAN
#endif

#if defined(__cpp_lib_bitops)
# define MOMO_CTZ32(value) std::countr_zero(value)
# define MOMO_CTZ64(value) std::countr_zero(value)
#elif defined(__GNUC__) || defined(__clang__)
# define MOMO_CTZ32(value) __builtin_ctz(value)
# define MOMO_CTZ64(value) __builtin_ctzll(value)
#endif

//#if defined(__GNUC__) || defined(__clang__)
//# define MOMO_PREFETCH(addr) __builtin_prefetch(addr)
//#endif

#define MOMO_ALIGNED_STORAGE(size, alignment) alignas(alignment) std::array<unsigned char, size>
#if defined(_MSC_VER) && (_MSC_VER < 1920 || defined(_M_CEE))	// C2719, C2711
# undef MOMO_ALIGNED_STORAGE
# define MOMO_ALIGNED_STORAGE(size, alignment) typename std::aligned_storage<size, alignment>::type
#endif

// `nullptr`, converted to the type `uintptr_t`
#define MOMO_NULL_UINTPTR reinterpret_cast<uintptr_t>(static_cast<void*>(nullptr))
#if defined(__clang__)
# undef MOMO_NULL_UINTPTR
# define MOMO_NULL_UINTPTR uintptr_t{0}
#endif

// One more pointer which doesn't point to anything but is not equal to `nullptr`
#define MOMO_INVALID_UINTPTR (MOMO_NULL_UINTPTR + 1)

#if defined(_MSC_VER)
// MSVC compiler has limited support for Empty Base Optimization by default
# define MOMO_EMPTY_BASES __declspec(empty_bases)
#else
# define MOMO_EMPTY_BASES
#endif

#define MOMO_ASSERT(expr) assert(expr)

#define MOMO_THROW(exception) throw exception

#define MOMO_CHECK_EXCEPTION(expr) \
	do { if (!(expr)) MOMO_THROW(std::invalid_argument(#expr)); } while (false)

#define MOMO_CAST_POINTER(ResObject, ptr, isWithinLifetime, isSingleObject) \
	((isWithinLifetime) ? std::launder(reinterpret_cast<ResObject*>(ptr)) : reinterpret_cast<ResObject*>(ptr))
#if !defined(__cpp_lib_launder) || (defined(_MSC_VER) && defined(_M_CEE))
# undef MOMO_CAST_POINTER
# define MOMO_CAST_POINTER(ResObject, ptr, isWithinLifetime, isSingleObject) reinterpret_cast<ResObject*>(ptr)
#endif

#ifdef __cpp_deduction_guides
# define MOMO_HAS_DEDUCTION_GUIDES
#endif

#ifdef __cpp_guaranteed_copy_elision
# define MOMO_HAS_GUARANTEED_COPY_ELISION
# if defined(_MSC_VER) && (_MSC_VER < 1930) && !defined(__clang__)
#  undef MOMO_HAS_GUARANTEED_COPY_ELISION
# endif
#endif

#if defined(__cpp_impl_three_way_comparison) && defined(__cpp_lib_three_way_comparison) \
	&& defined(__cpp_concepts)
# define MOMO_HAS_THREE_WAY_COMPARISON
#endif

#if defined(__cpp_lib_containers_ranges)
# define MOMO_HAS_CONTAINERS_RANGES
#endif

#ifdef __has_cpp_attribute
# if __has_cpp_attribute(nodiscard) \
	&& !(__cplusplus < 201703L && (defined(__GNUC__) || defined(__clang__)))
#  define MOMO_NODISCARD [[nodiscard]]
# endif
# if __has_cpp_attribute(deprecated) \
	&& !(__cplusplus < 201402L && (defined(__GNUC__) || defined(__clang__)))	// gcc 5 & 6
#  define MOMO_DEPRECATED [[deprecated]]
# endif
#endif
#ifndef MOMO_NODISCARD
# define MOMO_NODISCARD
#endif
#ifndef MOMO_DEPRECATED
# define MOMO_DEPRECATED
#endif

#endif // MOMO_INCLUDE_GUARD_USER_SETTINGS

/**********************************************************\

  momo/Settings.h

\**********************************************************/

#pragma once

//#define MOMO_USE_SAFE_MAP_BRACKETS
#define MOMO_USE_UNSAFE_MOVE_CONSTRUCTORS 1
#define MOMO_MAX_ALIGNMENT 1

#define MOMO_USE_NOEXCEPT
#define MOMO_USE_DELETE_FUNCS
#define MOMO_USE_VARIADIC_TEMPLATES
#define MOMO_USE_TYPE_ALIASES
#define MOMO_USE_INIT_LISTS

#if defined(_MSC_VER) && _MSC_VER < 1900
#undef MOMO_USE_NOEXCEPT
#endif

#if defined(_MSC_VER) && _MSC_VER < 1800
#undef MOMO_USE_SAFE_MAP_BRACKETS
#undef MOMO_USE_DELETE_FUNCS
#undef MOMO_USE_VARIADIC_TEMPLATES
#undef MOMO_USE_TYPE_ALIASES
#undef MOMO_USE_INIT_LISTS
#endif

#ifdef __GNUC__
#define MOMO_IS_TRIVIALLY_COPYABLE std::is_trivial
#else
#define MOMO_IS_TRIVIALLY_COPYABLE std::is_trivially_copyable
#endif

#if defined(_MSC_VER) && _MSC_VER < 1800
#define MOMO_REBIND_TO_CHAR_ALLOC(Allocator) \
	std::allocator_traits<Allocator>::template rebind_alloc<char>::other
#else
#define MOMO_REBIND_TO_CHAR_ALLOC(Allocator) \
	std::allocator_traits<Allocator>::template rebind_alloc<char>
#endif

#define MOMO_DEFAULT_HASH_BUCKET HashBucketLimP1<>

#if defined(_WIN32)
#define MOMO_DEFAULT_MEM_MANAGER MemManagerCpp
//#define MOMO_USE_MEM_MANAGER_WIN
//#define MOMO_DEFAULT_MEM_MANAGER MemManagerWin
#elif defined(__linux__)
#define MOMO_DEFAULT_MEM_MANAGER MemManagerC	// linux has fast realloc
#else
#define MOMO_DEFAULT_MEM_MANAGER MemManagerCpp
#endif

#ifdef _MSC_VER
#pragma warning (disable: 4127)	// conditional expression is constant
#pragma warning (disable: 4503)	// decorated name length exceeded, name was truncated
#pragma warning (disable: 4510)	// default constructor could not be generated
#pragma warning (disable: 4512)	// assignment operator could not be generated
#pragma warning (disable: 4610)	// struct can never be instantiated - user defined constructor required
#define _SCL_SECURE_NO_WARNINGS
#endif

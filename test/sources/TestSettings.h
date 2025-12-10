/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/TestSettings.h

\**********************************************************/

#pragma once

#if defined(__clang__)
# define TEST_CLANG
#elif defined(__GNUC__)	// && !defined(__clang__)
# define TEST_GCC
#elif defined(_MSC_VER)	// && !defined(__clang__)
# define TEST_MSVC
#endif

#if !defined(TEST_DISABLE_ALL)

#if !defined(TEST_DISABLE_SIMPLE)
# define TEST_SIMPLE_ARRAY
# define TEST_SIMPLE_HASH
# define TEST_SIMPLE_TREE
# define TEST_SIMPLE_MERGE
# define TEST_SIMPLE_DATA
# define TEST_SIMPLE_HASH_SORT
# define TEST_SIMPLE_MEM_POOL
# define TEST_SIMPLE_OBJECT
#endif

#if !defined(TEST_DISABLE_LIBCXX)
# define TEST_LIBCXX_ARRAY
# define TEST_LIBCXX_HASH_SET
# define TEST_LIBCXX_HASH_MAP
# define TEST_LIBCXX_HASH_MULTI_MAP
# define TEST_LIBCXX_TREE_SET
# define TEST_LIBCXX_TREE_MAP
# define TEST_LIBCXX_MERGE_SET
#endif

#if !defined(TEST_DISABLE_LIBCXX) && !defined(TEST_LIBCXX_VERSION)
# define TEST_LIBCXX_VERSION 20
#endif

#ifdef NDEBUG
# error "NDEBUG is defined!"
#endif

#endif // TEST_DISABLE_ALL

#define TEST_OLD_HASH_BUCKETS

//#define TEST_NATVIS
//#define TEST_SPEED_MAP

//#define MOMO_TEST_NO_EXCEPTIONS_RTTI
//#define MOMO_TEST_EXTRA_SETTINGS

#ifdef _WIN32
# define MOMO_USE_MEM_MANAGER_WIN
# define NOMINMAX
#endif

#ifdef MOMO_TEST_NO_EXCEPTIONS_RTTI
# define MOMO_DISABLE_EXCEPTIONS
# define MOMO_DISABLE_TYPE_INFO
#endif

#include "../../include/momo/UserSettings.h"

#ifdef MOMO_TEST_NO_EXCEPTIONS_RTTI
# undef MOMO_DEFAULT_CHECK_MODE
# define MOMO_DEFAULT_CHECK_MODE exception
#endif

#ifdef MOMO_TEST_EXTRA_SETTINGS

#undef MOMO_DEFAULT_EXTRA_CHECK_MODE
#define MOMO_DEFAULT_EXTRA_CHECK_MODE nothing

#undef MOMO_CHECK_ITERATOR_VERSION
#define MOMO_CHECK_ITERATOR_VERSION (checkMode != momo::CheckMode::assertion)

#undef MOMO_USE_HASH_TRAITS_STRING_SPECIALIZATION

#undef MOMO_USE_DEFAULT_MEM_MANAGER_IN_STD

#undef MOMO_USE_SSE2

#undef MOMO_ASSERT
#define MOMO_ASSERT(expr) void()

#undef MOMO_CATCH_ALL

#endif // MOMO_TEST_EXTRA_SETTINGS

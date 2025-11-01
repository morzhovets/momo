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

#ifndef TEST_DISABLE_ALL

#ifndef TEST_DISABLE_SIMPLE
# define TEST_SIMPLE_ARRAY
# define TEST_SIMPLE_HASH
# define TEST_SIMPLE_TREE
# define TEST_SIMPLE_MERGE
# define TEST_SIMPLE_DATA
# define TEST_SIMPLE_HASH_SORT
# define TEST_SIMPLE_MEM_POOL
# define TEST_SIMPLE_OBJECT
#endif

#ifndef TEST_DISABLE_LIBCXX
# define TEST_LIBCXX_ARRAY
# define TEST_LIBCXX_HASH_SET
# define TEST_LIBCXX_HASH_MAP
# define TEST_LIBCXX_HASH_MULTI_MAP
# define TEST_LIBCXX_TREE_SET
# define TEST_LIBCXX_TREE_MAP
#endif

//#define TEST_NATVIS

#define TEST_OLD_HASH_BUCKETS

#endif // TEST_DISABLE_ALL

#undef NDEBUG

#ifdef MOMO_TEST_NO_EXCEPTIONS_RTTI
# define MOMO_DISABLE_EXCEPTIONS
# define MOMO_DISABLE_TYPE_INFO
#endif

#ifdef _WIN32
# define MOMO_USE_MEM_MANAGER_WIN
# define NOMINMAX
#endif

#include "../../include/momo/UserSettings.h"

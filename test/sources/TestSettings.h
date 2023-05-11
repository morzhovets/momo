/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/TestSettings.h

\**********************************************************/

#pragma once

#undef NDEBUG

#if defined(__clang__)
#define TEST_CLANG
#elif defined(__GNUC__)	// && !defined(__clang__)
#define TEST_GCC
#elif defined(_MSC_VER)	// && !defined(__clang__)
#define TEST_MSVC
#endif

#define TEST_SIMPLE_ARRAY
#define TEST_SIMPLE_HASH
#define TEST_SIMPLE_TREE
#define TEST_SIMPLE_MERGE
#define TEST_SIMPLE_DATA
#define TEST_SIMPLE_HASH_SORT
#define TEST_SIMPLE_MEM_POOL

#define TEST_LIBCXX_ARRAY
#define TEST_LIBCXX_HASH_SET
#define TEST_LIBCXX_HASH_MAP
#define TEST_LIBCXX_HASH_MULTI_MAP
#define TEST_LIBCXX_TREE_SET
#define TEST_LIBCXX_TREE_MAP

//#define TEST_NATVIS

#define TEST_OLD_HASH_BUCKETS
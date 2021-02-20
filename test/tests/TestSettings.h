/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/TestSettings.h

\**********************************************************/

#pragma once

//#define TEST_SPEED_MAP

#ifndef TEST_SPEED_MAP

#define TEST_SIMPLE_ARRAY
#define TEST_SIMPLE_HASH
#define TEST_SIMPLE_TREE
#define TEST_SIMPLE_MERGE
#define TEST_SIMPLE_HASH_SORT
#define TEST_SIMPLE_DATA

#define TEST_LIBCXX_ARRAY
#define TEST_LIBCXX_HASH_SET
#define TEST_LIBCXX_HASH_MAP
#define TEST_LIBCXX_HASH_MULTI_MAP
#define TEST_LIBCXX_TREE_SET
#define TEST_LIBCXX_TREE_MAP

#undef NDEBUG

#endif

#define TEST_OLD_HASH_BUCKETS

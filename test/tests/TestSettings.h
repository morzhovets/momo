/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/TestSettings.h

\**********************************************************/

#pragma once

#ifdef NDEBUG
#define TEST_SPEED_MAP
#else
#define TEST_SIMPLE_ARRAY
#define TEST_SIMPLE_DATA
#define TEST_SIMPLE_HASH
#define TEST_SIMPLE_TREE
#define TEST_LIBCXX_ARRAY
#define TEST_LIBCXX_HASH_SET
#define TEST_LIBCXX_HASH_MAP
#define TEST_LIBCXX_HASH_MULTI_MAP
#define TEST_LIBCXX_TREE_SET
#define TEST_LIBCXX_TREE_MAP
#endif

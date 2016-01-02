/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  tests/TestSettings.h

\**********************************************************/

#pragma once

#ifdef NDEBUG
#define TEST_SPEED_HASH_MAP
#else
#define TEST_SIMPLE_ARRAY
#define TEST_SIMPLE_HASH
#define TEST_GOOGLE_HASH
#define TEST_LIBCXX_ARRAY
#define TEST_LIBCXX_HASH_SET
#define TEST_LIBCXX_HASH_MAP
#define TEST_LIBCXX_HASH_MULTI_MAP
#endif

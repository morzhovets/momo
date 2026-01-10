/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxSemiListTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_LIST

#include "LibcxxTester.h"

#include "../../include/momo/stdish/semi_list.h"

#include "../../include/momo/stdish/set.h"

namespace
{

using namespace libcxx_insert_range_seq;
using namespace libcxx_from_range_seq;

namespace libcxx_semi_list
{

namespace std
{
	using namespace ::std;

	template<typename TValue,
		typename TAllocator = std::allocator<TValue>>
	using list = momo::stdish::semi_list<TValue, TAllocator>;

	template<typename TKey>
	using set = momo::stdish::set<TKey>;
}

#define LIBCXX_TEST_CLASS momo::stdish::semi_list
#define LIBCXX_TEST_PREFIX "semi_list"
#include "libcxx20/ListTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_CLASS

} // namespace libcxx_semi_list

} // namespace

#endif // TEST_LIBCXX_LIST

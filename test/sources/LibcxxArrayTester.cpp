/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxArrayTester.cpp

\**********************************************************/

#include "pch.h"

#ifdef TEST_LIBCXX_ARRAY

#include "LibcxxTester.h"

#include "../../include/momo/stdish/vector.h"

namespace
{

using namespace libcxx_insert_range_seq;
using namespace libcxx_from_range_seq;

namespace libcxx_test_array_0
{

class LibcxxArraySettings : public momo::ArraySettings<0, false>
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};

namespace std
{
	using namespace ::std;

	template<typename TValue,
		typename TAllocator = std::allocator<TValue>>
	class MOMO_EMPTY_BASES vector
		: public momo::stdish::vector_adaptor<momo::Array<TValue,
			momo::MemManagerStd<TAllocator>,
			momo::ArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>,
			LibcxxArraySettings>>,
		public momo::internal::Swappable<vector>
	{
	private:
		typedef momo::stdish::vector_adaptor<momo::Array<TValue, momo::MemManagerStd<TAllocator>,
			momo::ArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>,
			LibcxxArraySettings>> VectorAdaptor;

	public:
		using VectorAdaptor::VectorAdaptor;

		using VectorAdaptor::operator=;
	};
}

#define LIBCXX_TEST_FAILURE
#define LIBCXX_TEST_ARRAY
#define LIBCXX_TEST_PREFIX "libcxx_test_array_0"
#include "libcxx/VectorTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_ARRAY
#undef LIBCXX_TEST_FAILURE

} // namespace libcxx_test_array_0

namespace libcxx_test_array_5
{

namespace std
{
	using namespace ::std;

	template<typename TValue,
		typename TAllocator = std::allocator<TValue>>
	using vector = momo::stdish::vector_intcap<5, TValue, TAllocator>;
}

#define LIBCXX_TEST_INTCAP_ARRAY
#define LIBCXX_TEST_PREFIX "libcxx_test_array_5"
#include "libcxx/VectorTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_INTCAP_ARRAY

} // namespace libcxx_test_array_5

} // namespace

#endif // TEST_LIBCXX_ARRAY

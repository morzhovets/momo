/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
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

namespace libcxx_array_0
{

class LibcxxArraySettings : public momo::ArraySettings<0, false>
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};

LIBCXX_NAMESPACE_STD_BEGIN
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
class vector : public momo::stdish::vector_adaptor<momo::ArrayCore<
	momo::ArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>, LibcxxArraySettings>>
{
private:
	typedef momo::stdish::vector_adaptor<momo::ArrayCore<
		momo::ArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>,
		LibcxxArraySettings>> VectorAdaptor;

public:
	using VectorAdaptor::VectorAdaptor;

	vector& operator=(std::initializer_list<typename VectorAdaptor::value_type> values)
	{
		VectorAdaptor::operator=(values);
		return *this;
	}

	friend void swap(vector& left, vector& right) noexcept
	{
		left.swap(right);
	}
};
LIBCXX_NAMESPACE_STD_END

#ifndef TEST_HAS_NO_EXCEPTIONS
# define LIBCXX_TEST_FAILURE
#endif
#define LIBCXX_TEST_ARRAY
#define LIBCXX_TEST_CLASS momo::stdish::vector
#define LIBCXX_TEST_PREFIX "array_0"
#include LIBCXX_HEADER(VectorTests.h)
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_CLASS
#undef LIBCXX_TEST_ARRAY
#undef LIBCXX_TEST_FAILURE

} // namespace libcxx_array_0

namespace libcxx_array_5
{

LIBCXX_NAMESPACE_STD_BEGIN
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
class vector : public momo::stdish::vector_intcap<5, TValue, TAllocator>
{
private:
	typedef momo::stdish::vector_intcap<5, TValue, TAllocator> VectorIntCap;

public:
	using VectorIntCap::VectorIntCap;

	vector& operator=(std::initializer_list<typename VectorIntCap::value_type> values)
	{
		VectorIntCap::operator=(values);
		return *this;
	}

	friend void swap(vector& left, vector& right) noexcept
	{
		left.swap(right);
	}
};
LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_INTCAP_ARRAY
#define LIBCXX_TEST_PREFIX "array_5"
#include LIBCXX_HEADER(VectorTests.h)
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_INTCAP_ARRAY

} // namespace libcxx_array_5

} // namespace

#endif // TEST_LIBCXX_ARRAY

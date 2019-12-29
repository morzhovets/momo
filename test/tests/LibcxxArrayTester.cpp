/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LibcxxArrayTester.cpp

\**********************************************************/

#include "pch.h"
#include "TestSettings.h"

#ifdef TEST_LIBCXX_ARRAY

#undef NDEBUG

#include "../../momo/Utility.h"

#include "LibcxxTester.h"

#include "../../momo/stdish/vector.h"
#include "../../momo/SegmentedArray.h"

template<typename TValue, typename TMemManager>
class LibcxxSegmentedArrayItemTraits : public momo::SegmentedArrayItemTraits<TValue, TMemManager>
{
public:
	static const bool isNothrowMoveConstructible =
		momo::internal::ObjectManager<TValue, TMemManager>::isNothrowMoveConstructible;
};

#define _LIBCPP_DEBUG 1
#define _LIBCPP_DEBUG_LEVEL 1

//#define LIBCPP_TEST_STACK_ALLOCATOR

#define LIBCXX_TEST_ARRAY
#define LIBCXX_TEST_PREFIX "libcxx_test_array_0"
namespace libcxx_test_array_0
{
class LibcxxArraySettings : public momo::ArraySettings<0, false>
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::vector<TValue, TAllocator,
	momo::Array<TValue, momo::MemManagerStd<TAllocator>,
		momo::ArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>, LibcxxArraySettings>>;
#include "LibcxxVectorTests.h"
}
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_ARRAY

//#undef LIBCPP_TEST_STACK_ALLOCATOR

#define LIBCXX_TEST_SEGMENTED_ARRAY

#define LIBCXX_TEST_PREFIX "libcxx_test_segmented_array_sqrt"
namespace libcxx_test_segmented_array_sqrt
{
class LibcxxSegmentedArraySettings
	: public momo::SegmentedArraySettings<momo::SegmentedArrayItemCountFunc::sqrt, 3>
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::vector<TValue, TAllocator,
	momo::SegmentedArray<TValue, momo::MemManagerStd<TAllocator>,
		LibcxxSegmentedArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>,
		LibcxxSegmentedArraySettings>>;
#include "LibcxxVectorTests.h"
}
#undef LIBCXX_TEST_PREFIX

#define LIBCXX_TEST_PREFIX "libcxx_test_segmented_array_cnst"
namespace libcxx_test_segmented_array_cnst
{
class LibcxxSegmentedArraySettings
	: public momo::SegmentedArraySettings<momo::SegmentedArrayItemCountFunc::cnst, 0>
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::vector<TValue, TAllocator,
	momo::SegmentedArray<TValue, momo::MemManagerStd<TAllocator>,
		LibcxxSegmentedArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>,
		LibcxxSegmentedArraySettings>>;
#include "LibcxxVectorTests.h"
}
#undef LIBCXX_TEST_PREFIX

#undef LIBCXX_TEST_SEGMENTED_ARRAY

#undef _LIBCPP_DEBUG
#undef _LIBCPP_DEBUG_LEVEL

#define LIBCXX_TEST_INTCAP_ARRAY
#define LIBCXX_TEST_PREFIX "libcxx_test_array_5"
namespace libcxx_test_array_5
{
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::vector_intcap<5, TValue, TAllocator>;
#include "LibcxxVectorTests.h"
}
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_INTCAP_ARRAY

#endif // TEST_LIBCXX_ARRAY

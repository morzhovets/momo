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
#include "../../include/momo/stdish/segmented_vector.h"

#undef _LIBCPP_DEBUG
#undef _LIBCPP_DEBUG_LEVEL
#define _LIBCPP_DEBUG 1
#define _LIBCPP_DEBUG_LEVEL 1

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

#define LIBCXX_TEST_SEGMENTED_ARRAY

#define LIBCXX_TEST_PREFIX "libcxx_test_segmented_array_sqrt_0"
namespace libcxx_test_segmented_array_sqrt_0
{
class LibcxxSegmentedArraySettings
	: public momo::SegmentedArraySettings<momo::SegmentedArrayItemCountFunc::sqrt, 0>
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::vector<TValue, TAllocator,
	momo::SegmentedArray<TValue, momo::MemManagerStd<TAllocator>,
		momo::SegmentedArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>,
		LibcxxSegmentedArraySettings>>;
#include "LibcxxVectorTests.h"
}
#undef LIBCXX_TEST_PREFIX

#define LIBCXX_TEST_PREFIX "libcxx_test_segmented_array_cnst_0"
namespace libcxx_test_segmented_array_cnst_0
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
		momo::SegmentedArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>,
		LibcxxSegmentedArraySettings>>;
#include "LibcxxVectorTests.h"
}
#undef LIBCXX_TEST_PREFIX

#define LIBCXX_TEST_PREFIX "libcxx_test_merge_array_0"
namespace libcxx_test_merge_array_0
{
class LibcxxMergeArraySettings : public momo::MergeArraySettings<0>
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::vector<TValue, TAllocator,
	momo::MergeArray<TValue, momo::MemManagerStd<TAllocator>,
		momo::MergeArrayItemTraits<TValue, momo::MemManagerStd<TAllocator>>,
		LibcxxMergeArraySettings>>;
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

#define LIBCXX_TEST_SEGMENTED_ARRAY

#define LIBCXX_TEST_PREFIX "libcxx_test_segmented_array_cnst"
namespace libcxx_test_segmented_array_cnst
{
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::segmented_vector<TValue, TAllocator>;
#include "LibcxxVectorTests.h"
}
#undef LIBCXX_TEST_PREFIX

#define LIBCXX_TEST_PREFIX "libcxx_test_segmented_array_sqrt"
namespace libcxx_test_segmented_array_sqrt
{
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::segmented_vector_sqrt<TValue, TAllocator>;
#include "LibcxxVectorTests.h"
}
#undef LIBCXX_TEST_PREFIX

#define LIBCXX_TEST_PREFIX "libcxx_test_merge_array"
namespace libcxx_test_merge_array
{
template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector = momo::stdish::merge_vector<TValue, TAllocator>;
#include "LibcxxVectorTests.h"
}
#undef LIBCXX_TEST_PREFIX

#undef LIBCXX_TEST_SEGMENTED_ARRAY

#endif // TEST_LIBCXX_ARRAY

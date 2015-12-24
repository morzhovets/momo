/**********************************************************\

  tests/LibcxxTreeSetTester.cpp

\**********************************************************/

#include "TestSettings.h"

#ifdef TEST_LIBCXX_TREE_SET

#undef NDEBUG

#include "../../momo/Utility.h"

#include "LibcxxTester.h"

#include "../../momo/stdish/set.h"

namespace
{

#define _LIBCPP_DEBUG 1
#define _LIBCPP_DEBUG_LEVEL 1

#define LIBCXX_TEST_PREFIX "libcxx_test_set"
struct LibcxxTreeSetSettings : public momo::TreeSetSettings
{
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
};
template<typename TKey,
	typename TLessFunc = std::less<TKey>,
	typename TAllocator = std::allocator<TKey>>
using set = momo::stdish::set<TKey, TLessFunc, TAllocator,
	momo::TreeSet<TKey, momo::TreeTraitsStd<TKey, TLessFunc>,
		momo::MemManagerStd<TAllocator>, momo::TreeSetItemTraits<TKey>, LibcxxTreeSetSettings>>;
#include "LibcxxSetTests.h"
#undef LIBCXX_TEST_PREFIX

#undef _LIBCPP_DEBUG
#undef _LIBCPP_DEBUG_LEVEL

} // namespace

#endif // TEST_LIBCXX_TREE_SET

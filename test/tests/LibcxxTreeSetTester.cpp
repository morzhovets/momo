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

#define LIBCXX_TEST_PREFIX "libcxx_test_set"
template<typename TKey,
	typename TLessFunc = std::less<TKey>,
	typename TAllocator = std::allocator<TKey>>
using set = momo::stdish::set<TKey, TLessFunc, TAllocator,
	momo::TreeSet<TKey, momo::TreeTraitsStd<TKey, TLessFunc, momo::TreeNodeSwp<3>>,
		momo::MemManagerStd<TAllocator>, momo::TreeSetItemTraits<TKey>, momo::TreeSetSettings>>;
#include "LibcxxSetTests.h"
#undef LIBCXX_TEST_PREFIX

} // namespace

#endif // TEST_LIBCXX_TREE_SET

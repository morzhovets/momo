/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxTester.h

\**********************************************************/

#pragma once

#include <iostream>
#include <cfloat>
#include <string>
#include <cmath>
#include <ranges>

//#define LIBCPP_HAS_BAD_NEWS_FOR_MOMO
//#define LIBCPP_TEST_MIN_ALLOCATOR

#define TEST_STD_VER 20

#include "libcxx/support/test_macros.h"

#include "libcxx/support/MoveOnly.h"
#include "libcxx/support/Copyable.h"
#include "libcxx/support/NotConstructible.h"
#include "libcxx/support/DefaultOnly.h"
#include "libcxx/support/Emplaceable.h"
#include "libcxx/support/Counter.h"
#include "libcxx/support/Moveable.h"
#include "libcxx/support/test_allocator.h"
#include "libcxx/support/test_iterators.h"
#include "libcxx/support/test_compare.h"
#include "libcxx/support/old_test_compare.h"
#include "libcxx/support/test_hash.h"
#include "libcxx/support/private_constructor.h"
#include "libcxx/support/is_transparent.h"
#include "libcxx/support/test_transparent_unordered.h"
#include "libcxx/support/test_comparisons.h"
#include "libcxx/support/emplace_constructible.h"
#include "libcxx/support/allocators.h"
#include "libcxx/support/container_test_types.h"
#include "libcxx/support/deduction_guides_sfinae_checks.h"
#include "libcxx/support/test_container_comparisons.h"
#include "libcxx/support/set_allocator_requirement_test_templates.h"
#include "libcxx/support/map_allocator_requirement_test_templates.h"
#include "libcxx/support/asan_testing.h"
#include "libcxx/support/min_allocator.h"

#include "../../include/momo/ObjectManager.h"

namespace momo
{
	template<int Dummy, conceptMemManager TMemManager>
	class ObjectRelocator<CopyInsertable<Dummy>, TMemManager>
	{
	public:
		typedef CopyInsertable<Dummy> Object;
		typedef TMemManager MemManager;

		static const bool isTriviallyRelocatable = false;
		static const bool isRelocatable = true;
		static const bool isNothrowRelocatable = true;

	public:
		static void Relocate(MemManager* /*srcMemManager*/, MemManager* /*dstMemManager*/,
			Object& srcObject, Object* dstObject) noexcept
		{
			std::construct_at(dstObject, srcObject.data);
			dstObject->copied_once = srcObject.copied_once;
			dstObject->constructed_under_allocator = srcObject.constructed_under_allocator;
			std::destroy_at(&srcObject);
		}
	};
}

template<typename It>
using input_iterator = cpp17_input_iterator<It>;

struct LibcppIntHash
{
	size_t operator()(int key) const noexcept
	{
		return static_cast<size_t>(key);
	}
};

#define LIBCPP_CATCH(expr) try { (void)(expr); assert(false); } catch (...) {}

#define LIBCXX_TEST_BEGIN(name) \
	namespace name \
	{ \
		template<typename Main> \
		int TestLibcxx(Main main) \
		{ \
			std::cout << LIBCXX_TEST_PREFIX << "_" << #name << ": " << std::flush; \
			if constexpr (std::is_same_v<Main, void (*)()>) \
				main(); \
			else \
				main(0, nullptr); \
			std::cout << "ok" << std::endl; \
			return 0; \
		}

#define LIBCXX_TEST_END \
		static int testLibcxx = TestLibcxx(&main); \
	}

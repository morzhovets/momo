/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxTester.h

\**********************************************************/

#pragma once

#include "libcxx/Support.h"

#include "../../include/momo/Version.h"
#include "../../include/momo/ObjectManager.h"

#include <iostream>

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

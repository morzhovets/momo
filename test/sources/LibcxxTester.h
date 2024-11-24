/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/LibcxxTester.h

\**********************************************************/

#pragma once

#include "TestSettings.h"

#include "../../include/momo/Version.h"
#include "../../include/momo/UserSettings.h"

#include <iostream>

#define LIBCXX_TO_STR(expr) #expr

#ifdef TEST_LIBCXX_NEW

#define LIBCXX_NAMESPACE_STD_BEGIN namespace std { using namespace ::std;

#define LIBCXX_NAMESPACE_STD_END }

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

#define LIBCXX_HEADER(header) LIBCXX_TO_STR(libcxx/header)

#else // TEST_LIBCXX_NEW

#define LIBCXX_NAMESPACE_STD_BEGIN

#define LIBCXX_NAMESPACE_STD_END

#define LIBCXX_TEST_BEGIN(name) \
	namespace name { \
		void main(); \
		static int testLibcxx = [] \
		{ \
			std::cout << LIBCXX_TEST_PREFIX << "_" << #name << ": " << std::flush; \
			main(); \
			std::cout << "ok" << std::endl; \
			return 0; \
		}();

#define LIBCXX_TEST_END }

#define LIBCXX_HEADER(header) LIBCXX_TO_STR(libcxx11/header)

#endif // TEST_LIBCXX_NEW

#include LIBCXX_HEADER(Support.h)

#ifdef TEST_LIBCXX_NEW

#include "../../include/momo/ObjectManager.h"

namespace momo
{
	template<int Dummy, typename TMemManager>
	class ObjectRelocator<CopyInsertable<Dummy>, TMemManager>
	{
	public:
		typedef CopyInsertable<Dummy> Object;
		typedef TMemManager MemManager;

		static const bool isTriviallyRelocatable = false;
		static const bool isNothrowRelocatable = true;

	public:
		static void Relocate(MemManager* /*memManager*/, Object& srcObject, Object* dstObject) noexcept
		{
			std::construct_at(dstObject, srcObject.data);
			dstObject->copied_once = srcObject.copied_once;
			dstObject->constructed_under_allocator = srcObject.constructed_under_allocator;
			std::destroy_at(&srcObject);
		}
	};
}

#endif // TEST_LIBCXX_NEW

/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  test/sources/SimpleObjectTester.cpp

\**********************************************************/

#include "pch.h"

#include "../../include/momo/ObjectManager.h"

namespace
{

struct ObjectDefCopy
{
	ObjectDefCopy(const ObjectDefCopy&) = default;

	~ObjectDefCopy() noexcept
	{
	}
};

MOMO_STATIC_ASSERT(!momo::IsTriviallyRelocatable<ObjectDefCopy>::value);
MOMO_STATIC_ASSERT((momo::ObjectRelocator<ObjectDefCopy, momo::MemManagerDefault>::isNothrowRelocatable));

struct ObjectDelCopy
{
	ObjectDelCopy(const ObjectDelCopy&) = delete;

	~ObjectDelCopy() noexcept
	{
	}
};

MOMO_STATIC_ASSERT(!momo::IsTriviallyRelocatable<ObjectDelCopy>::value);
MOMO_STATIC_ASSERT((!momo::ObjectRelocator<ObjectDelCopy, momo::MemManagerDefault>::isNothrowRelocatable));

struct ObjectCopy
{
	ObjectCopy(const ObjectCopy&)
	{
	}
};

MOMO_STATIC_ASSERT(!momo::IsTriviallyRelocatable<ObjectCopy>::value);
MOMO_STATIC_ASSERT((!momo::ObjectRelocator<ObjectCopy, momo::MemManagerDefault>::isNothrowRelocatable));

struct ObjectMove
{
	ObjectMove(const ObjectMove&&)
	{
	}
};

MOMO_STATIC_ASSERT(!momo::IsTriviallyRelocatable<ObjectMove>::value);
MOMO_STATIC_ASSERT((momo::ObjectRelocator<ObjectMove, momo::MemManagerDefault>::isNothrowRelocatable));

struct ObjectMoveCopy
{
	ObjectMoveCopy(ObjectMoveCopy&&)
	{
	}

	ObjectMoveCopy(const ObjectMoveCopy&) = default;
};

MOMO_STATIC_ASSERT(!momo::IsTriviallyRelocatable<ObjectMoveCopy>::value);
#if defined(TEST_MSVC)
MOMO_STATIC_ASSERT((!momo::ObjectRelocator<ObjectMoveCopy, momo::MemManagerDefault>::isNothrowRelocatable));
#else
MOMO_STATIC_ASSERT((momo::ObjectRelocator<ObjectMoveCopy, momo::MemManagerDefault>::isNothrowRelocatable));
#endif

} // namespace

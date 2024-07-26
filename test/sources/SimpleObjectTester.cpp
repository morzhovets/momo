/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
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

static_assert(!momo::IsTriviallyRelocatable<ObjectDefCopy>::value);
static_assert(momo::ObjectRelocator<ObjectDefCopy, momo::MemManagerDefault>::isNothrowRelocatable);

struct ObjectDelCopy
{
	ObjectDelCopy(const ObjectDelCopy&) = delete;

	~ObjectDelCopy() noexcept
	{
	}
};

static_assert(!momo::IsTriviallyRelocatable<ObjectDelCopy>::value);
static_assert(!momo::ObjectRelocator<ObjectDelCopy, momo::MemManagerDefault>::isNothrowRelocatable);

struct ObjectCopy
{
	ObjectCopy(const ObjectCopy&)
	{
	}
};

static_assert(!momo::IsTriviallyRelocatable<ObjectCopy>::value);
static_assert(!momo::ObjectRelocator<ObjectCopy, momo::MemManagerDefault>::isNothrowRelocatable);

struct ObjectMove
{
	ObjectMove(const ObjectMove&&)
	{
	}
};

static_assert(!momo::IsTriviallyRelocatable<ObjectMove>::value);
static_assert(momo::ObjectRelocator<ObjectMove, momo::MemManagerDefault>::isNothrowRelocatable);

struct ObjectMoveCopy
{
	ObjectMoveCopy(ObjectMoveCopy&&)
	{
	}

	ObjectMoveCopy(const ObjectMoveCopy&) = default;
};

static_assert(!momo::IsTriviallyRelocatable<ObjectMoveCopy>::value);
#if defined(TEST_MSVC)
static_assert(!momo::ObjectRelocator<ObjectMoveCopy, momo::MemManagerDefault>::isNothrowRelocatable);
#else
static_assert(momo::ObjectRelocator<ObjectMoveCopy, momo::MemManagerDefault>::isNothrowRelocatable);
#endif

} // namespace

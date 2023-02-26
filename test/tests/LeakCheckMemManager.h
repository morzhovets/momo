/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  tests/LeakCheckMemManager.h

\**********************************************************/

#pragma once

#include "../../momo/MemManager.h"

class LeakCheckMemManager
{
public:
	explicit LeakCheckMemManager() noexcept
		: mTotalSize(0)
	{
	}

	LeakCheckMemManager(LeakCheckMemManager&& memManager) noexcept
		: mTotalSize(std::exchange(memManager.mTotalSize, 0))
	{
	}

	LeakCheckMemManager(const LeakCheckMemManager& /*memManager*/) noexcept
		: LeakCheckMemManager()
	{
	}

	~LeakCheckMemManager() noexcept
	{
		assert(mTotalSize == 0);
	}

	LeakCheckMemManager& operator=(const LeakCheckMemManager&) = delete;

	[[nodiscard]] void* Allocate(size_t size)
	{
		void* res = momo::MemManagerDefault().Allocate(size);
		mTotalSize += size;
		return res;
	}

	void Deallocate(void* ptr, size_t size) noexcept
	{
		momo::MemManagerDefault().Deallocate(ptr, size);
		mTotalSize -= size;
	}

private:
	size_t mTotalSize;
};

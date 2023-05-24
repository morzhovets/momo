/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/MemManagerDict.h

  namespace momo:
    class MemManagerDictSettings
    class MemManagerDict

\**********************************************************/

#pragma once

#include "MemManager.h"
#include "TreeMap.h"

namespace momo
{

class MemManagerDictSettings
{
public:
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;

	typedef TreeNodeDefault DictTreeNode;
};

template<conceptMemManager TBaseMemManager = MemManagerDefault,
	typename TSettings = MemManagerDictSettings>
class MemManagerDict
{
public:
	typedef TBaseMemManager BaseMemManager;
	typedef TSettings Settings;

private:
	typedef TreeTraits<void*, false, typename Settings::DictTreeNode> DictTreeTraits;

	typedef TreeMap<void*, size_t, DictTreeTraits, BaseMemManager,
		TreeMapKeyValueTraits<void*, size_t, BaseMemManager>,
		internal::NestedTreeMapSettings> Dict;

public:
	explicit MemManagerDict(BaseMemManager baseMemManager = BaseMemManager())
		: mDict(DictTreeTraits(), std::move(baseMemManager))
	{
	}

	MemManagerDict(MemManagerDict&& memManager) noexcept
		: mDict(std::move(memManager.mDict))
	{
	}

	MemManagerDict(const MemManagerDict& memManager)
		: MemManagerDict(memManager.GetBaseMemManager())
	{
	}

	~MemManagerDict() noexcept
	{
		MOMO_EXTRA_CHECK(mDict.IsEmpty());
	}

	MemManagerDict& operator=(const MemManagerDict&) = delete;

	const BaseMemManager& GetBaseMemManager() const noexcept
	{
		return mDict.GetMemManager();
	}

	BaseMemManager& GetBaseMemManager() noexcept
	{
		return mDict.GetMemManager();
	}

	[[nodiscard]] void* Allocate(size_t size)
	{
		BaseMemManager& baseMemManager = GetBaseMemManager();
		void* ptr = baseMemManager.Allocate(size);
		try
		{
			mDict.Insert(ptr, size);
		}
		catch (...)
		{
			baseMemManager.Deallocate(ptr, size);
			throw;
		}
		return ptr;
	}

	void Deallocate(void* ptr, size_t size) noexcept
	{
		typename Dict::ConstIterator iter = mDict.Find(ptr);
		MOMO_EXTRA_CHECK(iter != mDict.GetEnd());
		MOMO_EXTRA_CHECK(iter->value == size);
		mDict.Remove(iter);
		GetBaseMemManager().Deallocate(ptr, size);
	}

private:
	Dict mDict;
};

} // namespace momo

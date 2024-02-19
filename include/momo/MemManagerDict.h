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
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;

	typedef TreeNodeDefault BlockDictTreeNode;
};

template<typename TBaseMemManager = MemManagerDefault,
	typename TSettings = MemManagerDictSettings>
class MemManagerDict
{
public:
	typedef TBaseMemManager BaseMemManager;
	typedef TSettings Settings;

	static const size_t ptrUsefulBitCount =
		internal::MemManagerProxy<BaseMemManager>::ptrUsefulBitCount;

private:
	typedef TreeTraits<void*, false, typename Settings::BlockDictTreeNode> BlockDictTreeTraits;

	typedef TreeMap<void*, size_t, BlockDictTreeTraits, BaseMemManager,
		TreeMapKeyValueTraits<void*, size_t, BaseMemManager>,
		internal::NestedTreeMapSettings> BlockDict;

public:
	explicit MemManagerDict(BaseMemManager baseMemManager = BaseMemManager())
		: mBlockDict(BlockDictTreeTraits(), std::move(baseMemManager))
	{
	}

	MemManagerDict(MemManagerDict&&) = default;

	MemManagerDict(const MemManagerDict& memManager)
		: MemManagerDict(memManager.GetBaseMemManager())
	{
	}

	~MemManagerDict() noexcept
	{
		MOMO_EXTRA_CHECK(mBlockDict.IsEmpty());
	}

	MemManagerDict& operator=(MemManagerDict&&) = default;

	MemManagerDict& operator=(const MemManagerDict&) = delete;

	const BaseMemManager& GetBaseMemManager() const noexcept
	{
		return mBlockDict.GetMemManager();
	}

	BaseMemManager& GetBaseMemManager() noexcept
	{
		return mBlockDict.GetMemManager();
	}

	MOMO_NODISCARD void* Allocate(size_t size)
	{
		MOMO_CHECK(size > 0);
		BaseMemManager& baseMemManager = GetBaseMemManager();
		void* ptr = baseMemManager.Allocate(size);
		try
		{
			mBlockDict.Insert(ptr, size);
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
		auto iter = mBlockDict.Find(ptr);
		MOMO_EXTRA_CHECK(iter != mBlockDict.GetEnd());
		MOMO_EXTRA_CHECK(iter->value == size);
		mBlockDict.Remove(iter);
		GetBaseMemManager().Deallocate(ptr, size);
	}

	void* FindBlock(void* ptr, size_t* resSize = nullptr) const noexcept
	{
		auto iter = mBlockDict.GetUpperBound(ptr);
		if (iter != mBlockDict.GetBegin())
		{
			auto ref = *std::prev(iter);
			if (internal::PtrCaster::ToUInt(ptr) <
				internal::PtrCaster::ToUInt(ref.key) + uintptr_t{ref.value})
			{
				if (resSize != nullptr)
					*resSize = ref.value;
				return ref.key;
			}
		}
		return nullptr;
	}

private:
	BlockDict mBlockDict;
};

} // namespace momo

/**********************************************************\

  momo/TreeTraits.h

  namespace momo:
    class TreeTraits
    class TreeTraitsStd

\**********************************************************/

#pragma once

#include "details/TreeNode.h"

namespace momo
{

template<typename TKey>
class TreeTraits
{
public:
	typedef TKey Key;

	static const size_t nodeCapacity = 8;

public:
	TreeTraits() MOMO_NOEXCEPT
	{
	}

	bool Less(const Key& key1, const Key& key2) const
	{
		return std::less<Key>()(key1, key2);
	}
};

template<typename TKey,
	typename TLessFunc = std::less<TKey>>
class TreeTraitsStd
{
public:
	typedef TKey Key;
	typedef TLessFunc LessFunc;

	static const size_t nodeCapacity = 8;

public:
	explicit TreeTraitsStd(const LessFunc& lessFunc = LessFunc()) MOMO_NOEXCEPT
		: mLessFunc(lessFunc)
	{
	}

	bool Less(const Key& key1, const Key& key2) const
	{
		return mLessFunc(key1, key2);
	}

	const LessFunc& GetLessFunc() const MOMO_NOEXCEPT
	{
		return mLessFunc;
	}

private:
	LessFunc mLessFunc;
};

} // namespace momo

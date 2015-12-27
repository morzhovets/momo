/**********************************************************\

  momo/TreeTraits.h

  namespace momo:
    struct TreeNodeDefault
    class TreeTraits
    class TreeTraitsStd

\**********************************************************/

#pragma once

#include "details/TreeNodeSwp.h"
#include "details/TreeNodePrm.h"

namespace momo
{

typedef TreeNodeSwp<32> TreeNodeDefault;

template<typename TKey,
	typename TTreeNode = TreeNodeDefault>
class TreeTraits
{
public:
	typedef TKey Key;
	typedef TTreeNode TreeNode;

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
	typename TLessFunc = std::less<TKey>,
	typename TTreeNode = TreeNodeDefault>
class TreeTraitsStd
{
public:
	typedef TKey Key;
	typedef TLessFunc LessFunc;
	typedef TTreeNode TreeNode;

public:
	explicit TreeTraitsStd(const LessFunc& lessFunc = LessFunc())
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

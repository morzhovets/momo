/**********************************************************\

  momo/TreeTraits.h

  namespace momo:
    struct IsFastComparable
    struct TreeNodeDefault
    class TreeTraits
    class TreeTraitsStd

\**********************************************************/

#pragma once

#include "details/TreeNode.h"

namespace momo
{

template<typename Key>
struct IsFastComparable : public internal::BoolConstant<MOMO_IS_FAST_COMPARABLE(Key)>
{
};

typedef MOMO_DEFAULT_TREE_NODE TreeNodeDefault;

template<typename TKey,
	typename TTreeNode = TreeNodeDefault,
	bool tUseLinearSearch = IsFastComparable<TKey>::value>
class TreeTraits
{
public:
	typedef TKey Key;
	typedef TTreeNode TreeNode;

	static const bool useLinearSearch = tUseLinearSearch;

public:
	TreeTraits() MOMO_NOEXCEPT
	{
	}

	bool IsLess(const Key& key1, const Key& key2) const
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

	static const bool useLinearSearch =
		std::is_same<LessFunc, std::less<Key>>::value && IsFastComparable<Key>::value;

public:
	explicit TreeTraitsStd(const LessFunc& lessFunc = LessFunc())
		: mLessFunc(lessFunc)
	{
	}

	bool IsLess(const Key& key1, const Key& key2) const
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

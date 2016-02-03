/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

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
	bool tUseLinearSearch = IsFastComparable<TKey>::value,
	typename TKeyArg = void>
class TreeTraits
{
public:
	typedef TKey Key;
	typedef TTreeNode TreeNode;
	typedef TKeyArg KeyArg;

	static const bool useLinearSearch = tUseLinearSearch;

	template<typename KeyArg>
	using IsValidKeyArg = internal::BoolConstant<!std::is_same<KeyArg, void>::value
		&& std::is_same<KeyArg, typename TreeTraits::KeyArg>::value>;

public:
	TreeTraits() MOMO_NOEXCEPT
	{
	}

	bool IsLess(const Key& key1, const Key& key2) const
	{
		return std::less<Key>()(key1, key2);
	}

	template<typename KeyArg1, typename KeyArg2>
	bool IsLess(const KeyArg1& key1, const KeyArg2& key2) const
	{
		MOMO_STATIC_ASSERT((std::is_same<Key, KeyArg1>::value) || IsValidKeyArg<KeyArg1>::value);
		MOMO_STATIC_ASSERT((std::is_same<Key, KeyArg2>::value) || IsValidKeyArg<KeyArg2>::value);
		return key1 < key2;
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

	template<typename KeyArg>
	using IsValidKeyArg = std::true_type;

public:
	explicit TreeTraitsStd(const LessFunc& lessFunc = LessFunc())
		: mLessFunc(lessFunc)
	{
	}

	template<typename KeyArg1, typename KeyArg2>
	bool IsLess(const KeyArg1& key1, const KeyArg2& key2) const
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

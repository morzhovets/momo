/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/TreeTraits.h

  namespace momo:
    class TreeNodeDefault
    concept conceptTreeTraits
    class TreeTraits
    class TreeTraitsStd

\**********************************************************/

#pragma once

#include "KeyUtility.h"
#include "details/TreeNode.h"

namespace momo
{

namespace internal
{
	template<typename Key, typename KeyArg>
	concept conceptTreeTraitsValidKeyArg =
		requires (const Key& key1, const KeyArg& key2)
		{
			{ key1 < key2 } -> std::convertible_to<bool>;
			{ key2 < key1 } -> std::convertible_to<bool>;
		};
}

typedef MOMO_DEFAULT_TREE_NODE TreeNodeDefault;

template<typename TreeTraits, typename Key>
concept conceptTreeTraits =
	std::copy_constructible<TreeTraits> &&
	requires (const TreeTraits& treeTraits, const Key& key)
	{
		typename std::bool_constant<TreeTraits::multiKey>;
		typename std::bool_constant<TreeTraits::useLinearSearch>;
		typename std::bool_constant<TreeTraits::template IsValidKeyArg<Key>::value>;
		{ treeTraits.GetSplitItemIndex(size_t{}, size_t{}) } -> std::same_as<size_t>;
		{ treeTraits.IsLess(key, key) } -> std::same_as<bool>;
	};

template<conceptObject TKey,
	bool tMultiKey = false,
	typename TTreeNode = TreeNodeDefault,
	bool tUseLinearSearch = IsFastComparable<TKey>::value>
class TreeTraits
{
public:
	typedef TKey Key;
	typedef TTreeNode TreeNode;

	static const bool multiKey = tMultiKey;
	static const bool useLinearSearch = tUseLinearSearch;

	template<typename ItemTraits>
	using Node = typename TreeNode::template Node<ItemTraits>;

	template<typename KeyArg>
	using IsValidKeyArg = std::bool_constant<internal::conceptTreeTraitsValidKeyArg<Key, KeyArg>>;

public:
	explicit TreeTraits() noexcept = default;

	size_t GetSplitItemIndex(size_t itemCount, size_t newItemIndex) const noexcept
	{
		return TreeNode::GetSplitItemIndex(itemCount, newItemIndex);
	}

	template<typename KeyArg1, typename KeyArg2>
	static bool IsLess(const KeyArg1& key1, const KeyArg2& key2)
		requires requires { { key1 < key2 } -> std::convertible_to<bool>; }
	{
		return std::less<>()(key1, key2);
	}
};

template<conceptObject TKey,
	internal::conceptCopyableLessFunc<TKey> TLessFunc = std::less<TKey>,
	bool tMultiKey = false,
	typename TTreeNode = TreeNodeDefault>
class TreeTraitsStd
{
public:
	typedef TKey Key;
	typedef TLessFunc LessFunc;
	typedef TTreeNode TreeNode;

	static const bool multiKey = tMultiKey;

	static const bool useLinearSearch = IsFastComparable<Key>::value
		&& (std::is_same_v<LessFunc, std::less<Key>> || std::is_same_v<LessFunc, std::less<>>);

	template<typename ItemTraits>
	using Node = typename TreeNode::template Node<ItemTraits>;

	template<typename KeyArg>
	using IsValidKeyArg = std::bool_constant<internal::conceptTransparent<LessFunc>>;

private:
	static const bool staticIsLess = std::is_empty_v<LessFunc> &&
		std::is_trivially_default_constructible_v<LessFunc>;

public:
	explicit TreeTraitsStd(const LessFunc& lessFunc = LessFunc())
		: mLessFunc(lessFunc)
	{
	}

	size_t GetSplitItemIndex(size_t itemCount, size_t newItemIndex) const noexcept
	{
		return TreeNode::GetSplitItemIndex(itemCount, newItemIndex);
	}

	template<typename KeyArg1, typename KeyArg2>
	static bool IsLess(const KeyArg1& key1, const KeyArg2& key2)
		requires (staticIsLess)
	{
		return LessFunc()(key1, key2);
	}

	template<typename KeyArg1, typename KeyArg2>
	bool IsLess(const KeyArg1& key1, const KeyArg2& key2) const
		requires (!staticIsLess)
	{
		return mLessFunc(key1, key2);
	}

	const LessFunc& GetLessFunc() const noexcept
	{
		return mLessFunc;
	}

private:
	[[no_unique_address]] LessFunc mLessFunc;
};

} // namespace momo

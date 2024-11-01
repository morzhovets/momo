/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/TreeTraits.h

  namespace momo:
    struct IsFastComparable
    class TreeNodeDefault
    class TreeTraits
    class TreeTraitsStd

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_TREE_TRAITS
#define MOMO_INCLUDE_GUARD_TREE_TRAITS

#include "details/TreeNode.h"

namespace momo
{

namespace internal
{
	template<typename Key, typename KeyArg,
		typename = void>
	struct TreeTraitsIsValidKeyArg : public std::false_type
	{
	};

	template<typename Key, typename KeyArg>
	struct TreeTraitsIsValidKeyArg<Key, KeyArg, EnableIf<
		std::is_convertible<decltype(std::declval<const Key&>() < std::declval<const KeyArg&>()), bool>::value &&
		std::is_convertible<decltype(std::declval<const KeyArg&>() < std::declval<const Key&>()), bool>::value>>
		: public std::true_type
	{
	};

	template<typename LessFunc,
		typename = void>
	struct TreeTraitsStdIsValidKeyArg : public std::false_type
	{
	};

	template<typename LessFunc>
	struct TreeTraitsStdIsValidKeyArg<LessFunc, Void<typename LessFunc::is_transparent>>
		: public std::true_type
	{
	};

	template<typename TreeTraits, typename ItemTraits,
		typename = void>
	struct TreeTraitsNodeSelector
	{
		MOMO_DEPRECATED typedef typename TreeTraits::TreeNode::template Node<ItemTraits> Node;
	};

	template<typename TreeTraits, typename ItemTraits>
	struct TreeTraitsNodeSelector<TreeTraits, ItemTraits,
		Void<typename TreeTraits::template Node<ItemTraits>>>
	{
		typedef typename TreeTraits::template Node<ItemTraits> Node;
	};
}

template<typename Key>
struct IsFastComparable : public internal::BoolConstant<MOMO_IS_FAST_COMPARABLE(Key)>
{
};

typedef MOMO_DEFAULT_TREE_NODE TreeNodeDefault;

template<typename TKey,
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
	using IsValidKeyArg = internal::TreeTraitsIsValidKeyArg<Key, KeyArg>;

public:
	explicit TreeTraits() noexcept
	{
	}

	size_t GetSplitItemIndex(size_t itemCount, size_t newItemIndex) const noexcept
	{
		return TreeNode::GetSplitItemIndex(itemCount, newItemIndex);
	}

	template<typename KeyArg1, typename KeyArg2>
	bool IsLess(const KeyArg1& key1, const KeyArg2& key2) const
	{
		return key1 < key2;
	}

	template<typename KeyArg1, typename KeyArg2>
	bool IsLess(KeyArg1* key1, KeyArg2* key2) const noexcept
	{
		return std::less<const void*>()(key1, key2);
	}
};

template<typename TKey,
	typename TLessFunc = std::less<TKey>,
	bool tMultiKey = false,
	typename TTreeNode = TreeNodeDefault>
class TreeTraitsStd
{
public:
	typedef TKey Key;
	typedef TLessFunc LessFunc;
	typedef TTreeNode TreeNode;

	static const bool multiKey = tMultiKey;

	static const bool useLinearSearch =
		std::is_same<LessFunc, std::less<Key>>::value && IsFastComparable<Key>::value;

	template<typename ItemTraits>
	using Node = typename TreeNode::template Node<ItemTraits>;

	template<typename KeyArg>
	using IsValidKeyArg = internal::TreeTraitsStdIsValidKeyArg<LessFunc>;

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
	bool IsLess(const KeyArg1& key1, const KeyArg2& key2) const
	{
		return mLessFunc(key1, key2);
	}

	const LessFunc& GetLessFunc() const noexcept
	{
		return mLessFunc;
	}

private:
	/*[[no_unique_address]]*/ LessFunc mLessFunc;
};

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_TREE_TRAITS

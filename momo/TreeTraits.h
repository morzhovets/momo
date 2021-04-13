/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/TreeTraits.h

  namespace momo:
    struct IsFastComparable
    class TreeNodeDefault
    concept conceptTreeTraits
    class TreeTraits
    class TreeTraitsStd

\**********************************************************/

#pragma once

#include "details/TreeNode.h"

namespace momo
{

namespace internal
{
	template<typename LessFunc, typename Key>
	concept conceptLessFunc = std::strict_weak_order<const LessFunc&, const Key&, const Key&>;

	template<typename Key, typename KeyArg,
		typename = void>
	struct TreeTraitsIsValidKeyArg : public std::false_type
	{
	};

	template<typename Key, typename KeyArg>
	struct TreeTraitsIsValidKeyArg<Key, KeyArg, std::enable_if_t<
		std::is_convertible_v<decltype(std::declval<const Key&>() < std::declval<const KeyArg&>()), bool> &&
		std::is_convertible_v<decltype(std::declval<const KeyArg&>() < std::declval<const Key&>()), bool>>>
		: public std::true_type
	{
	};

	template<typename LessFunc,
		typename = void>
	struct TreeTraitsStdIsValidKeyArg : public std::false_type
	{
	};

	template<typename LessFunc>
	struct TreeTraitsStdIsValidKeyArg<LessFunc, std::void_t<typename LessFunc::is_transparent>>
		: public std::true_type
	{
	};
}

template<typename Key>
struct IsFastComparable : public std::bool_constant<MOMO_IS_FAST_COMPARABLE(Key)>
{
};

typedef MOMO_DEFAULT_TREE_NODE TreeNodeDefault;

template<typename TreeTraits, typename Key>
concept conceptTreeTraits =
	std::is_nothrow_destructible_v<TreeTraits> &&
	std::is_copy_constructible_v<TreeTraits> &&
	requires (const TreeTraits& treeTraits, const Key& key)
	{
		typename TreeTraits::TreeNode;
		{ TreeTraits::multiKey } -> std::convertible_to<bool>;
		{ TreeTraits::useLinearSearch } -> std::convertible_to<bool>;
		{ TreeTraits::template IsValidKeyArg<Key>::value } -> std::convertible_to<bool>;
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

	template<typename KeyArg>
	using IsValidKeyArg = internal::TreeTraitsIsValidKeyArg<Key, KeyArg>;

public:
	explicit TreeTraits() noexcept
	{
	}

	template<typename KeyArg1, typename KeyArg2>
	requires requires (const KeyArg1& key1, const KeyArg2& key2)
		{ { key1 < key2 } -> std::convertible_to<bool>; }
	bool IsLess(const KeyArg1& key1, const KeyArg2& key2) const
	{
		return std::less<>()(key1, key2);
	}
};

template<conceptObject TKey,
	typename TLessFunc = std::less<TKey>,
	bool tMultiKey = false,
	typename TTreeNode = TreeNodeDefault>
requires internal::conceptLessFunc<TLessFunc, TKey> && std::is_copy_constructible_v<TLessFunc>
class TreeTraitsStd
{
public:
	typedef TKey Key;
	typedef TLessFunc LessFunc;
	typedef TTreeNode TreeNode;

	static const bool multiKey = tMultiKey;

	static const bool useLinearSearch = IsFastComparable<Key>::value
		&& (std::is_same_v<LessFunc, std::less<Key>> || std::is_same_v<LessFunc, std::less<>>);

	template<typename KeyArg>
	using IsValidKeyArg = internal::TreeTraitsStdIsValidKeyArg<LessFunc>;

public:
	explicit TreeTraitsStd(const LessFunc& lessFunc = LessFunc())
		: mLessFunc(lessFunc)
	{
	}

	template<typename KeyArg1, typename KeyArg2>
	requires (std::is_same_v<Key, KeyArg1> || IsValidKeyArg<KeyArg1>::value) &&
		(std::is_same_v<Key, KeyArg2> || IsValidKeyArg<KeyArg2>::value)
	bool IsLess(const KeyArg1& key1, const KeyArg2& key2) const
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

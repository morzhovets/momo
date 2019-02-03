/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/TreeTraits.h

  namespace momo:
    struct IsFastComparable
    class TreeNodeDefault
    class TreeTraits
    class TreeTraitsStd

\**********************************************************/

#pragma once

#include "details/TreeNode.h"

namespace momo
{

namespace internal
{
	template<typename Key, typename KeyArg,
		typename = bool, typename = bool>
	struct TreeTraitsIsValidKeyArg : public std::false_type
	{
	};

	template<typename Key, typename KeyArg>
	struct TreeTraitsIsValidKeyArg<Key, KeyArg,
		decltype(std::declval<const Key&>() < std::declval<const KeyArg&>()),
		decltype(std::declval<const KeyArg&>() < std::declval<const Key&>())>
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

	template<typename KeyArg>
	using IsValidKeyArg = internal::TreeTraitsIsValidKeyArg<Key, KeyArg>;

public:
	explicit TreeTraits() noexcept
	{
	}

	template<typename KeyArg1, typename KeyArg2>
	bool IsLess(const KeyArg1& key1, const KeyArg2& key2) const
	{
		MOMO_STATIC_ASSERT((std::is_same<Key, KeyArg1>::value) || IsValidKeyArg<KeyArg1>::value);
		MOMO_STATIC_ASSERT((std::is_same<Key, KeyArg2>::value) || IsValidKeyArg<KeyArg2>::value);
		return key1 < key2;
	}

	template<typename KeyArg1, typename KeyArg2>
	bool IsLess(KeyArg1* key1, KeyArg2* key2) const noexcept
	{
		MOMO_STATIC_ASSERT((std::is_same<Key, KeyArg1*>::value) || IsValidKeyArg<KeyArg1*>::value);
		MOMO_STATIC_ASSERT((std::is_same<Key, KeyArg2*>::value) || IsValidKeyArg<KeyArg2*>::value);
		return std::less<const void*>()(key1, key2);
	}
};

template<typename TKey,
	typename TLessFunc = std::less<TKey>,
	bool tMultiKey = false,
	typename TTreeNode = TreeNodeDefault>
class TreeTraitsStd : private TLessFunc
{
public:
	typedef TKey Key;
	typedef TLessFunc LessFunc;
	typedef TTreeNode TreeNode;

	static const bool multiKey = tMultiKey;

	static const bool useLinearSearch =
		std::is_same<LessFunc, std::less<Key>>::value && IsFastComparable<Key>::value;

	template<typename KeyArg>
	using IsValidKeyArg = internal::TreeTraitsStdIsValidKeyArg<LessFunc>;

public:
	explicit TreeTraitsStd(const LessFunc& lessFunc = LessFunc())
		: LessFunc(lessFunc)
	{
	}

	template<typename KeyArg1, typename KeyArg2>
	bool IsLess(const KeyArg1& key1, const KeyArg2& key2) const
	{
		MOMO_STATIC_ASSERT((std::is_same<Key, KeyArg1>::value) || IsValidKeyArg<KeyArg1>::value);
		MOMO_STATIC_ASSERT((std::is_same<Key, KeyArg2>::value) || IsValidKeyArg<KeyArg2>::value);
		return LessFunc::operator()(key1, key2);
	}

	const LessFunc& GetLessFunc() const noexcept
	{
		return *this;
	}
};

} // namespace momo

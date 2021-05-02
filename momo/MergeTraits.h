/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/MergeTraits.h

  namespace momo:
    concept conceptMergeTraits
    class MergeTraits

\**********************************************************/

#pragma once

#include "MergeArray.h"

namespace momo
{

template<typename MergeTraits, typename Key>
concept conceptMergeTraits =
	std::copy_constructible<MergeTraits> &&
	requires (const MergeTraits& mergeTraits, const Key& key)
	{
		typename MergeTraits::MergeArraySettings;
		typename std::bool_constant<MergeTraits::isNothrowComparable>;
		{ mergeTraits.IsLess(key, key) } -> std::same_as<bool>;
		{ mergeTraits.IsEqual(key, key) } -> std::same_as<bool>;
	};

template<conceptObject TKey,
	bool tIsNothrowComparable = noexcept(std::declval<const TKey&>() < std::declval<const TKey&>()),
	typename TMergeArraySettings = MergeArraySettings<4>>
class MergeTraits
{
public:
	typedef TKey Key;
	typedef TMergeArraySettings MergeArraySettings;

	static const bool isNothrowComparable = tIsNothrowComparable;

public:
	explicit MergeTraits() noexcept
	{
	}

	bool IsLess(const Key& key1, const Key& key2) const noexcept(isNothrowComparable)
		requires requires { { key1 < key2 } -> std::convertible_to<bool>; }
	{
		return std::less<>()(key1, key2);
	}

	bool IsEqual(const Key& key1, const Key& key2) const
		requires requires { { key1 == key2 } -> std::convertible_to<bool>; }
	{
		return key1 == key2;
	}
};

} // namespace momo

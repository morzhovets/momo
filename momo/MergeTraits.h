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
	std::is_nothrow_destructible_v<MergeTraits> &&
	std::is_copy_constructible_v<MergeTraits> &&
	requires (const MergeTraits& mergeTraits, const Key& key)
	{
		typename MergeTraits::MergeArraySettings;
		{ MergeTraits::isNothrowComparable } -> std::convertible_to<bool>;
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
	{
		return std::less<>()(key1, key2);
	}

	bool IsEqual(const Key& key1, const Key& key2) const
	{
		return key1 == key2;
	}
};

} // namespace momo

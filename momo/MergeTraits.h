/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/MergeTraits.h

  namespace momo:
    class MergeTraits

\**********************************************************/

#pragma once

#include "Utility.h"

namespace momo
{

template<typename TKey,
	bool tIsNothrowComparable = noexcept(std::declval<const TKey&>() < std::declval<const TKey&>()),
	size_t tLogInitialItemCount = 5>
class MergeTraits
{
public:
	typedef TKey Key;

	static const bool isNothrowComparable = tIsNothrowComparable;
	static const size_t logInitialItemCount = tLogInitialItemCount;

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

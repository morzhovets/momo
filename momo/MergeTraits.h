/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/MergeTraits.h

  namespace momo:
    enum class MergeTraitsFunc
    concept conceptMergeTraits
    class MergeTraits
    class MergeTraitsHash

\**********************************************************/

#pragma once

#include "KeyUtility.h"
#include "MergeArray.h"

namespace momo
{

enum class MergeTraitsFunc
{
	hash = 0,
	lessNothrow = 1,
	lessThrow = 2,
};

template<typename MergeTraits, typename Key>
concept conceptMergeTraits =
	std::copy_constructible<MergeTraits> &&
	requires (const MergeTraits& mergeTraits, const Key& key)
	{
		typename MergeTraits::MergeArraySettings;
		typename std::integral_constant<MergeTraitsFunc, MergeTraits::func>;
		//{ mergeTraits.GetHashCode(key) } -> std::same_as<size_t>;
		//{ mergeTraits.IsLess(key, key) } -> std::same_as<bool>;
		{ mergeTraits.IsEqual(key, key) } -> std::same_as<bool>;
	};

template<conceptObject TKey,
	MergeTraitsFunc tFunc = noexcept(std::declval<const TKey&>() < std::declval<const TKey&>())
		? MergeTraitsFunc::lessNothrow : MergeTraitsFunc::lessThrow,
	typename TMergeArraySettings = MergeArraySettings<4>>
class MergeTraits
{
public:
	typedef TKey Key;
	typedef TMergeArraySettings MergeArraySettings;

	static const MergeTraitsFunc func = tFunc;

public:
	explicit MergeTraits() = default;

	size_t GetHashCode(const Key& key) const
		requires requires { { HashCoder<Key>()(key) } -> std::convertible_to<size_t>; }
	{
		return HashCoder<Key>()(key);
	}

	bool IsLess(const Key& key1, const Key& key2) const noexcept(func == MergeTraitsFunc::lessNothrow)
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

template<conceptObject TKey>
using MergeTraitsHash = MergeTraits<TKey, MergeTraitsFunc::hash>;

} // namespace momo

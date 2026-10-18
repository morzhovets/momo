/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/KeyUtility.h

  namespace momo:
    struct IsFastNothrowHashable
    struct IsFastComparable
    struct HashCoder

\**********************************************************/

#pragma once

#include "FunctionUtility.h"

namespace momo
{

namespace internal
{
	template<typename EqualComparer, typename Key,
		typename KeyArg = Key, typename... KeyArgs>
	concept conceptEqualComparer = conceptPredicate<EqualComparer, const Key&, const KeyArg&> &&
		(conceptPredicate<EqualComparer, const Key&, const KeyArgs&> && ...);

	template<typename LessComparer, typename Key>
	concept conceptLessComparer = conceptPredicate<LessComparer, const Key&, const Key&>;

	template<typename Hasher, typename Key>
	concept conceptHasher = conceptConstFunctor<Hasher, size_t, const Key&>;

	template<typename EqualComparer, typename Key>
	concept conceptCopyableEqualComparer = conceptEqualComparer<EqualComparer, Key> &&
		std::copy_constructible<EqualComparer>;

	template<typename LessComparer, typename Key>
	concept conceptCopyableLessComparer = conceptLessComparer<LessComparer, Key> &&
		std::copy_constructible<LessComparer>;

	template<typename Hasher, typename Key>
	concept conceptCopyableHasher = conceptHasher<Hasher, Key> &&
		std::copy_constructible<Hasher>;

	template<typename Hasher>
	concept conceptAvalanching = std::is_base_of_v<std::true_type, typename Hasher::is_avalanching> ||
		std::is_void_v<typename Hasher::is_avalanching>;

	template<typename Predicate>
	concept conceptTransparent = requires { typename Predicate::is_transparent; };
}

template<typename Key>
struct IsFastNothrowHashable : public std::bool_constant<MOMO_IS_FAST_NOTHROW_HASHABLE(Key)>
{
};

template<typename Key>
struct IsFastComparable : public std::bool_constant<MOMO_IS_FAST_COMPARABLE(Key)>
{
};

template<typename Key,
	typename Result = size_t>
struct HashCoder : private std::hash<Key>
{
	Result operator()(const Key& key) const
		noexcept(std::is_nothrow_invocable_v<const std::hash<Key>&, const Key&>)
	{
		return static_cast<Result>(std::hash<Key>::operator()(key));
	}
};

#ifdef MOMO_HASH_CODER
template<typename Key>
struct HashCoder<Key, decltype(MOMO_HASH_CODER(std::declval<const Key&>()))>
{
	decltype(auto) operator()(const Key& key) const noexcept(noexcept(MOMO_HASH_CODER(key)))
	{
		return MOMO_HASH_CODER(key);
	}
};
#endif

namespace internal
{
	template<typename Key>
	requires (std::three_way_comparable<std::tuple<const Key&>>)
	using TieThreeComparer = decltype([] (const Key& key1, const Key& key2)
		{ return std::tie(key1) <=> std::tie(key2); });

	class HashMixer
	{
	public:
		static size_t MixFast(size_t hashCode) noexcept
		{
			uint64_t hashCode64 = hashCode;
			if constexpr (sizeof(size_t) == 4)
			{
				hashCode64 *= 0x9E3779B9u;	// golden ratio
				hashCode64 ^= hashCode64 >> 32;
			}
			else
			{
				hashCode64 *= 0x9E3779B97F4A7C15ull;	// golden ratio
			}
			return static_cast<size_t>(hashCode64);
		}
	};

	class StrHasher
	{
	public:
		static consteval uint64_t GetHashCode64(const char* str) noexcept
		{
			// Fowler-Noll-Vo hash function (1a)
			uint64_t hashCode64 = 0xCBF29CE484222325ull;
			for (const char* p = str; *p != '\0'; ++p)
				hashCode64 = (hashCode64 ^ uint64_t{static_cast<unsigned char>(*p)}) * 0x100000001B3ull;
			return hashCode64;
		}
	};
}

} // namespace momo

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

#ifdef __SSE4_2__
# include <nmmintrin.h>
#endif

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
		static size_t MixHashCode(size_t hashCode) noexcept
		{
			if constexpr (sizeof(size_t) == 4)
			{
				uint64_t hashCode64 = uint64_t{hashCode} * 0xE817FB2Dull;
				return static_cast<size_t>(hashCode64) ^ static_cast<size_t>(hashCode64 >> 32);
			}
			else
			{
				uint64_t hashCode64 = hashCode;
#ifdef __SSE4_2__
				hashCode64 ^= _mm_crc32_u64(0, hashCode64);
#else
				// MurmurHash3
				hashCode64 ^= hashCode64 >> 33;
				hashCode64 *= 0xFF51AFD7ED558CCDull;
				hashCode64 ^= hashCode64 >> 33;
				hashCode64 *= 0xC4CEB9FE1A85EC53ull;
				hashCode64 ^= hashCode64 >> 33;
#endif
				return static_cast<size_t>(hashCode64);
			}
		}

		template<typename Hasher, typename Key>
		static size_t GetMixedHashCode(const Hasher& hasher, const Key& key)
			noexcept(noexcept(hasher(key)))
		{
			size_t hashCode = hasher(key);
			if constexpr (!conceptAvalanching<Hasher>)
				hashCode = MixHashCode(hashCode);
			return hashCode;
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

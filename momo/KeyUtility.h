/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/KeyUtility.h

  namespace momo:
    struct IsFastNothrowHashable
    struct IsFastComparable
    struct HashCoder

\**********************************************************/

#pragma once

#include "Utility.h"

namespace momo
{

namespace internal
{
	template<typename HashFunc, typename Key>
	concept conceptHashFunc =
		requires (const HashFunc& hashFunc, const Key& key)
			{ { hashFunc(key) } -> std::convertible_to<size_t>; };

	template<typename EqualFunc, typename Key1,
		typename Key2 = Key1>
	concept conceptEqualFunc = std::predicate<const EqualFunc&, const Key1&, const Key2&>;

	template<typename LessFunc, typename Key>
	concept conceptLessFunc = std::strict_weak_order<const LessFunc&, const Key&, const Key&>;

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
#ifndef __GNUC__	//?
		noexcept(noexcept(std::hash<Key>::operator()(key)))
#endif
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
	class StrHasher
	{
	private:
		static const uint64_t fnvBasis64 = 14695981039346656037ull;
		static const uint64_t fnvPrime64 = 1099511628211ull;

	public:
		// Fowler-Noll-Vo hash function (1a)
		static constexpr uint64_t GetHashCode64(const char* str) noexcept
		{
			uint64_t res = fnvBasis64;
			for (const char* p = str; *p != '\0'; ++p)
				res = (res ^ uint64_t{static_cast<unsigned char>(*p)}) * fnvPrime64;
			return res;
		}
	};
}

} // namespace momo

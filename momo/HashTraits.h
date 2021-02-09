/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/HashTraits.h

  namespace momo:
    struct IsFastNothrowHashable
    struct HashCoder
    class HashBucketDefault
    class HashBucketOpenDefault
    class HashTraits
    class HashTraitsOpen
    class HashTraitsStd

\**********************************************************/

#pragma once

#include "details/HashBucketLimP4.h"
#include "details/HashBucketOpen2N2.h"
#include "details/HashBucketOpen8.h"

#ifdef MOMO_INCLUDE_OLD_HASH_BUCKETS
#include "details/HashBucketLim4.h"
#include "details/HashBucketLimP.h"
#include "details/HashBucketLimP1.h"
#include "details/HashBucketUnlimP.h"
#include "details/HashBucketOne.h"
#include "details/HashBucketOpenN1.h"
#endif

#ifdef MOMO_USE_HASH_TRAITS_STRING_SPECIALIZATION
#include <string_view>
#endif

namespace momo
{

namespace internal
{
#ifdef MOMO_USE_HASH_TRAITS_STRING_SPECIALIZATION
	template<typename TString,
		typename TChar = typename TString::value_type,
		typename TCharTraits = typename TString::traits_type,
		typename TStringView = std::basic_string_view<TChar, TCharTraits>,
		typename = std::enable_if_t<std::is_convertible_v<const TString&, TStringView>
			&& !std::is_same_v<TString, TStringView>>>
	struct HashTraitsStringViewSelector
	{
		typedef TString String;
		typedef TStringView StringView;
	};
#endif

	template<typename HashFunc, typename EqualFunc,
		typename = void>
	struct HashTraitsStdIsValidKeyArg : public std::false_type
	{
	};

	template<typename HashFunc, typename EqualFunc>
	struct HashTraitsStdIsValidKeyArg<HashFunc, EqualFunc,
		std::void_t<typename HashFunc::is_transparent, typename EqualFunc::is_transparent>>
		: public std::true_type
	{
	};
}

template<typename Key>
struct IsFastNothrowHashable : public std::bool_constant<MOMO_IS_FAST_NOTHROW_HASHABLE(Key)>
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

typedef MOMO_DEFAULT_HASH_BUCKET HashBucketDefault;

typedef MOMO_DEFAULT_HASH_BUCKET_OPEN HashBucketOpenDefault;

template<typename TKey,
	typename THashBucket = HashBucketDefault,
	typename TKeyArgBase = TKey>
class HashTraits
{
public:
	typedef TKey Key;
	typedef THashBucket HashBucket;
	typedef TKeyArgBase KeyArgBase;

	template<typename KeyArg>
	using IsValidKeyArg = std::conditional_t<std::is_same_v<KeyArgBase, Key>,
		std::false_type, std::is_convertible<const KeyArg&, const KeyArgBase&>>;	//?

	static const bool isFastNothrowHashable = IsFastNothrowHashable<KeyArgBase>::value;

public:
	explicit HashTraits() noexcept
	{
	}

	size_t CalcCapacity(size_t bucketCount, size_t bucketMaxItemCount) const noexcept
	{
		return HashBucket::CalcCapacity(bucketCount, bucketMaxItemCount);
	}

	size_t GetBucketCountShift(size_t bucketCount, size_t bucketMaxItemCount) const noexcept
	{
		return HashBucket::GetBucketCountShift(bucketCount, bucketMaxItemCount);
	}

	size_t GetLogStartBucketCount() const noexcept
	{
		return HashBucket::logStartBucketCount;
	}

	template<typename KeyArg>
	size_t GetHashCode(const KeyArg& key) const
	{
		static_assert((std::is_convertible_v<const KeyArg&, const KeyArgBase&>));
		//static_assert(std::is_empty_v<HashCoder<KeyArgBase>>);
		return HashCoder<KeyArgBase>()(key);
	}

	template<typename KeyArg1, typename KeyArg2>
	bool IsEqual(const KeyArg1& key1, const KeyArg2& key2) const
	{
		static_assert((std::is_convertible_v<const KeyArg1&, const KeyArgBase&>));
		static_assert((std::is_convertible_v<const KeyArg2&, const KeyArgBase&>));
		return std::equal_to<KeyArgBase>()(key1, key2);
	}
};

#ifdef MOMO_USE_HASH_TRAITS_STRING_SPECIALIZATION
template<typename StringKey, typename HashBucket>
class HashTraits<StringKey, HashBucket,
	typename internal::HashTraitsStringViewSelector<StringKey>::String>
	: public HashTraits<StringKey, HashBucket,
		typename internal::HashTraitsStringViewSelector<StringKey>::StringView>
{
public:
	explicit HashTraits() noexcept
	{
	}
};
#endif

template<typename TKey>
using HashTraitsOpen = HashTraits<TKey, HashBucketOpenDefault>;

template<typename TKey,
	typename THashFunc = HashCoder<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename THashBucket = HashBucketDefault>
class HashTraitsStd
{
public:
	typedef TKey Key;
	typedef THashFunc HashFunc;
	typedef TEqualFunc EqualFunc;
	typedef THashBucket HashBucket;

	template<typename KeyArg>
	using IsValidKeyArg = internal::HashTraitsStdIsValidKeyArg<HashFunc, EqualFunc>;

	static const bool isFastNothrowHashable = IsFastNothrowHashable<Key>::value &&
		(std::is_same_v<HashFunc, HashCoder<Key>> || std::is_same_v<HashFunc, std::hash<Key>>);

public:
	explicit HashTraitsStd(size_t startBucketCount = size_t{1} << HashBucket::logStartBucketCount,
		const HashFunc& hashFunc = HashFunc(),
		const EqualFunc& equalFunc = EqualFunc())
		: mHashFunc(hashFunc),
		mEqualFunc(equalFunc),
		mMaxLoadFactor(0.0)
	{
		startBucketCount = std::minmax(startBucketCount, size_t{8}).second;
		mLogStartBucketCount = static_cast<uint8_t>(internal::UIntMath<>::Log2(startBucketCount - 1)) + 1;
	}

	HashTraitsStd(const HashTraitsStd& hashTraits, float maxLoadFactor)
		: mHashFunc(hashTraits.mHashFunc),
		mEqualFunc(hashTraits.mEqualFunc),
		mLogStartBucketCount(hashTraits.mLogStartBucketCount),
		mMaxLoadFactor(maxLoadFactor)
	{
	}

	size_t CalcCapacity(size_t bucketCount, size_t bucketMaxItemCount) const noexcept
	{
		//return static_cast<size_t>(static_cast<float>(bucketCount)
		//	* GetMaxLoadFactor(bucketMaxItemCount));
		if (mMaxLoadFactor > 0.0)
			return static_cast<size_t>(static_cast<float>(bucketCount) * mMaxLoadFactor);
		return HashBucket::CalcCapacity(bucketCount, bucketMaxItemCount);
	}

	size_t GetBucketCountShift(size_t bucketCount, size_t bucketMaxItemCount) const noexcept
	{
		return HashBucket::GetBucketCountShift(bucketCount, bucketMaxItemCount);
	}

	size_t GetLogStartBucketCount() const noexcept
	{
		return size_t{mLogStartBucketCount};
	}

	template<typename KeyArg>
	size_t GetHashCode(const KeyArg& key) const
	{
		static_assert((std::is_same_v<Key, KeyArg>) || IsValidKeyArg<KeyArg>::value);
		return mHashFunc(key);
	}

	template<typename KeyArg1, typename KeyArg2>
	bool IsEqual(const KeyArg1& key1, const KeyArg2& key2) const
	{
		static_assert((std::is_same_v<Key, KeyArg1>) || IsValidKeyArg<KeyArg1>::value);
		static_assert((std::is_same_v<Key, KeyArg2>));
		return mEqualFunc(key1, key2);
	}

	const HashFunc& GetHashFunc() const noexcept
	{
		return mHashFunc;
	}

	const EqualFunc& GetEqualFunc() const noexcept
	{
		return mEqualFunc;
	}

	float GetMaxLoadFactor(size_t bucketMaxItemCount) const noexcept
	{
		if (mMaxLoadFactor > 0.0)
			return mMaxLoadFactor;
		static const size_t testBucketCount = 1 << 16;
		size_t testCapacity = HashBucket::CalcCapacity(testBucketCount, bucketMaxItemCount);
		MOMO_ASSERT(testCapacity > 0);
		return static_cast<float>(testCapacity) / static_cast<float>(testBucketCount);
	}

private:
	HashFunc mHashFunc;
	EqualFunc mEqualFunc;
	uint8_t mLogStartBucketCount;
	float mMaxLoadFactor;
};

} // namespace momo

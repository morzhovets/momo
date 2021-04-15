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
    concept conceptHashTraits
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
	template<typename HashFunc, typename Key>
	concept conceptHashFunc =
		requires (const HashFunc& hashFunc, const Key& key)
			{ { hashFunc(key) } -> std::convertible_to<size_t>; };

	template<typename EqualFunc, typename Key>
	concept conceptEqualFunc = std::equivalence_relation<const EqualFunc&, const Key&, const Key&>;

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

template<typename HashTraits, typename Key>
concept conceptHashTraits =
	std::copy_constructible<HashTraits> &&
	requires (const HashTraits& hashTraits, const Key& key)
	{
		typename HashTraits::HashBucket;
		{ HashTraits::template IsValidKeyArg<Key>::value } -> std::convertible_to<bool>;
		{ HashTraits::isFastNothrowHashable } -> std::convertible_to<bool>;
		{ hashTraits.CalcCapacity(size_t{}, size_t{}) } -> std::same_as<size_t>;
		{ hashTraits.GetBucketCountShift(size_t{}, size_t{}) } -> std::same_as<size_t>;
		{ hashTraits.GetLogStartBucketCount() } -> std::same_as<size_t>;
		{ hashTraits.GetHashCode(key) } -> std::same_as<size_t>;
		{ hashTraits.IsEqual(key, key) } -> std::same_as<bool>;
	};

template<conceptObject TKey,
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
	requires std::is_convertible_v<const KeyArg&, const KeyArgBase&> &&
		internal::conceptHashFunc<HashCoder<KeyArgBase>, KeyArgBase> &&
		std::is_default_constructible_v<HashCoder<KeyArgBase>> // && std::is_empty_v<HashCoder<KeyArgBase>>
	size_t GetHashCode(const KeyArg& key) const
	{
		return HashCoder<KeyArgBase>()(static_cast<const KeyArgBase&>(key));
	}

	template<typename KeyArg1, typename KeyArg2>
	requires std::is_convertible_v<const KeyArg1&, const KeyArgBase&> &&
		std::is_convertible_v<const KeyArg2&, const KeyArgBase&> &&
		requires (const KeyArgBase& key) { { key == key } -> std::convertible_to<bool>; }
	bool IsEqual(const KeyArg1& key1, const KeyArg2& key2) const
	{
		return static_cast<const KeyArgBase&>(key1) == static_cast<const KeyArgBase&>(key2);
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

template<conceptObject TKey>
using HashTraitsOpen = HashTraits<TKey, HashBucketOpenDefault>;

template<conceptObject TKey,
	typename THashFunc = HashCoder<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename THashBucket = HashBucketDefault>
requires internal::conceptHashFunc<THashFunc, TKey> && std::copy_constructible<THashFunc> &&
	internal::conceptEqualFunc<TEqualFunc, TKey> && std::copy_constructible<TEqualFunc>
class HashTraitsStd
{
public:
	typedef TKey Key;
	typedef THashFunc HashFunc;
	typedef TEqualFunc EqualFunc;
	typedef THashBucket HashBucket;

	template<typename KeyArg>
	using IsValidKeyArg = std::bool_constant<
		internal::conceptTransparent<HashFunc> && internal::conceptTransparent<EqualFunc>>;

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
		mLogStartBucketCount = static_cast<uint8_t>(std::bit_width(startBucketCount - 1));
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
	requires std::is_same_v<Key, KeyArg> || IsValidKeyArg<KeyArg>::value
	size_t GetHashCode(const KeyArg& key) const
	{
		return mHashFunc(key);
	}

	template<typename KeyArg1, typename KeyArg2>
	requires (std::is_same_v<Key, KeyArg1> || IsValidKeyArg<KeyArg1>::value) &&
		(std::is_same_v<Key, KeyArg2> || IsValidKeyArg<KeyArg2>::value)
	bool IsEqual(const KeyArg1& key1, const KeyArg2& key2) const
	{
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

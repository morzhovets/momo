/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/HashTraits.h

  namespace momo:
    class HashBucketDefault
    class HashBucketOpenDefault
    struct HashTraitsKeyArgBaseSelector
    concept conceptHashTraits
    class HashTraits
    class HashTraitsOpen
    class HashTraitsStd

\**********************************************************/

#pragma once

#include "KeyUtility.h"
#include "details/HashBucketLimP4.h"
#include "details/HashBucketOpen2N2.h"
#include "details/HashBucketOpen8.h"

#ifdef MOMO_INCLUDE_OLD_HASH_BUCKETS
# include "details/HashBucketLim4.h"
# include "details/HashBucketLimP.h"
# include "details/HashBucketLimP1.h"
# include "details/HashBucketUnlimP.h"
# include "details/HashBucketOne.h"
# include "details/HashBucketOpenN1.h"
#endif

#ifdef MOMO_USE_HASH_TRAITS_STRING_SPECIALIZATION
# include <string_view>
#endif

namespace momo
{

typedef MOMO_DEFAULT_HASH_BUCKET HashBucketDefault;

typedef MOMO_DEFAULT_HASH_BUCKET_OPEN HashBucketOpenDefault;

template<typename Key>
struct HashTraitsKeyArgBaseSelector
{
	typedef Key KeyArgBase;
};

#ifdef MOMO_USE_HASH_TRAITS_STRING_SPECIALIZATION
template<typename Key>
requires std::is_convertible_v<const Key&,
	const std::basic_string_view<typename Key::value_type, typename Key::traits_type>&>
struct HashTraitsKeyArgBaseSelector<Key>
{
	typedef std::basic_string_view<typename Key::value_type, typename Key::traits_type> KeyArgBase;
};
#endif

template<typename HashTraits, typename Key>
concept conceptHashTraits =
	std::copy_constructible<HashTraits> &&
	requires (const HashTraits& hashTraits, const Key& key)
	{
		typename std::bool_constant<HashTraits::template IsValidKeyArg<Key>::value>;
		typename std::bool_constant<HashTraits::isFastNothrowHashable>;
		{ hashTraits.CalcCapacity(size_t{}, size_t{}) } -> std::same_as<size_t>;
		{ hashTraits.GetBucketCountShift(size_t{}, size_t{}) } -> std::same_as<size_t>;
		{ hashTraits.GetLogStartBucketCount() } -> std::same_as<size_t>;
		{ hashTraits.GetHashCode(key) } -> std::same_as<size_t>;
		{ hashTraits.IsEqual(key, key) } -> std::same_as<bool>;
	};

template<conceptObject TKey,
	typename THashBucket = HashBucketDefault,
	typename TKeyArgBase = typename HashTraitsKeyArgBaseSelector<TKey>::KeyArgBase>
requires std::is_convertible_v<const TKey&, const TKeyArgBase&>
class HashTraits
{
public:
	typedef TKey Key;
	typedef THashBucket HashBucket;
	typedef TKeyArgBase KeyArgBase;

	static const bool isFastNothrowHashable = IsFastNothrowHashable<KeyArgBase>::value;

	template<typename ItemTraits>
	using Bucket = typename HashBucket::template Bucket<ItemTraits, !isFastNothrowHashable>;

	template<typename KeyArg>
	using IsValidKeyArg = std::conditional_t<std::is_same_v<KeyArgBase, Key>,
		std::false_type, std::is_convertible<const KeyArg&, const KeyArgBase&>>;	//?

public:
	explicit HashTraits() noexcept = default;

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
		requires requires { { HashCoder<KeyArgBase>()(static_cast<const KeyArgBase&>(key)) }
			-> std::convertible_to<size_t>; }
	{
		return HashCoder<KeyArgBase>()(static_cast<const KeyArgBase&>(key));
	}

	template<typename KeyArg1, typename KeyArg2>
	static bool IsEqual(const KeyArg1& key1, const KeyArg2& key2)
		requires requires { { static_cast<const KeyArgBase&>(key1) == static_cast<const KeyArgBase&>(key2) }
			-> std::convertible_to<bool>; }
	{
		return static_cast<const KeyArgBase&>(key1) == static_cast<const KeyArgBase&>(key2);
	}
};

template<conceptObject TKey>
using HashTraitsOpen = HashTraits<TKey, HashBucketOpenDefault>;

template<conceptObject TKey,
	internal::conceptCopyableHashFunc<TKey> THashFunc = HashCoder<TKey>,
	internal::conceptCopyableEqualFunc<TKey> TEqualFunc = std::equal_to<TKey>,
	typename THashBucket = HashBucketDefault>
class HashTraitsStd
{
public:
	typedef TKey Key;
	typedef THashFunc HashFunc;
	typedef TEqualFunc EqualFunc;
	typedef THashBucket HashBucket;

	static const bool isFastNothrowHashable = IsFastNothrowHashable<Key>::value &&
		(std::is_same_v<HashFunc, HashCoder<Key>> || std::is_same_v<HashFunc, std::hash<Key>>);

	template<typename ItemTraits>
	using Bucket = typename HashBucket::template Bucket<ItemTraits, !isFastNothrowHashable>;

	template<typename KeyArg>
	using IsValidKeyArg = std::bool_constant<
		internal::conceptTransparent<HashFunc> && internal::conceptTransparent<EqualFunc>>;

private:
	static const bool staticIsEqual = std::is_empty_v<EqualFunc> &&
		std::is_trivially_default_constructible_v<EqualFunc>;

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
	size_t GetHashCode(const KeyArg& key) const
	{
		return mHashFunc(key);
	}

	template<typename KeyArg1, typename KeyArg2>
	static bool IsEqual(const KeyArg1& key1, const KeyArg2& key2)
		requires (staticIsEqual)
	{
		return EqualFunc()(key1, key2);
	}

	template<typename KeyArg1, typename KeyArg2>
	bool IsEqual(const KeyArg1& key1, const KeyArg2& key2) const
		requires (!staticIsEqual)
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

/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

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
#include "details/HashBucketOneIA.h"
#include "details/HashBucketOneI1.h"
#include "details/HashBucketOneI.h"
#include "details/HashBucketOpen2N.h"
#include "details/HashBucketOpenN1.h"
#endif

namespace momo
{

namespace internal
{
	template<typename TKey, typename THashFunc, typename TEqualFunc,
		typename = void>
	class HashTraitsStdBase
	{
	public:
		typedef TKey Key;
		typedef THashFunc HashFunc;
		typedef TEqualFunc EqualFunc;

	protected:
		static const bool isValidKeyArg = false;
	};

#ifdef MOMO_USE_UNORDERED_HETEROGENEOUS_LOOKUP
	template<typename TKey, typename THashFunc, typename TEqualFunc>
	class HashTraitsStdBase<TKey, THashFunc, TEqualFunc,
		Void<typename THashFunc::transparent_key_equal>>
	{
	public:
		typedef TKey Key;
		typedef THashFunc HashFunc;
		typedef typename THashFunc::transparent_key_equal EqualFunc;

	protected:
		static const bool isValidKeyArg = true;

	private:
		typedef typename EqualFunc::is_transparent IsTransparent;

		MOMO_STATIC_ASSERT((std::is_same<TEqualFunc, EqualFunc>::value
			|| std::is_same<TEqualFunc, std::equal_to<Key>>::value));
	};
#endif
}

template<typename Key>
struct IsFastNothrowHashable : public internal::BoolConstant<MOMO_IS_FAST_NOTHROW_HASHABLE(Key)>
{
};

template<typename Key,
	typename Result = size_t>
struct HashCoder : public std::hash<Key>
{
};

#ifdef MOMO_HASH_CODER
template<typename Key>
struct HashCoder<Key, decltype(MOMO_HASH_CODER(std::declval<const Key&>()))>
{
	decltype(MOMO_HASH_CODER(std::declval<const Key&>())) operator()(const Key& key) const
		noexcept(IsFastNothrowHashable<Key>::value)
	{
		return MOMO_HASH_CODER(key);
	}
};
#endif

typedef MOMO_DEFAULT_HASH_BUCKET HashBucketDefault;

typedef MOMO_DEFAULT_HASH_BUCKET_OPEN HashBucketOpenDefault;

template<typename TKey,
	typename THashBucket = HashBucketDefault>
class HashTraits
{
public:
	typedef TKey Key;
	typedef THashBucket HashBucket;

	template<typename KeyArg>
	using IsValidKeyArg = std::is_convertible<const KeyArg&, const Key&>;

	static const bool isFastNothrowHashable = IsFastNothrowHashable<Key>::value;

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
		MOMO_STATIC_ASSERT((std::is_same<Key, KeyArg>::value) || IsValidKeyArg<KeyArg>::value);
		//MOMO_STATIC_ASSERT(std::is_empty<HashCoder<Key>>::value);
		return HashCoder<Key>()(key);
	}

	template<typename KeyArg1, typename KeyArg2>
	bool IsEqual(const KeyArg1& key1, const KeyArg2& key2) const
	{
		MOMO_STATIC_ASSERT((std::is_same<Key, KeyArg1>::value) || IsValidKeyArg<KeyArg1>::value);
		MOMO_STATIC_ASSERT((std::is_same<Key, KeyArg2>::value) || IsValidKeyArg<KeyArg2>::value);
		return std::equal_to<Key>()(key1, key2);
	}
};

template<typename TKey>
using HashTraitsOpen = HashTraits<TKey, HashBucketOpenDefault>;

template<typename TKey,
	typename THashFunc = HashCoder<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename THashBucket = HashBucketDefault>
class HashTraitsStd : public internal::HashTraitsStdBase<TKey, THashFunc, TEqualFunc>
{
private:
	typedef internal::HashTraitsStdBase<TKey, THashFunc, TEqualFunc> HashTraitsStdBase;

public:
	using typename HashTraitsStdBase::Key;
	using typename HashTraitsStdBase::HashFunc;
	using typename HashTraitsStdBase::EqualFunc;

	typedef THashBucket HashBucket;

	template<typename KeyArg>
	using IsValidKeyArg = internal::BoolConstant<HashTraitsStdBase::isValidKeyArg>;

	static const bool isFastNothrowHashable = IsFastNothrowHashable<Key>::value
		&& (std::is_same<HashFunc, HashCoder<Key>>::value
		|| std::is_same<HashFunc, std::hash<Key>>::value);

public:
	explicit HashTraitsStd(size_t startBucketCount = (size_t)1 << HashBucket::logStartBucketCount,
		const HashFunc& hashFunc = HashFunc(),
		const EqualFunc& equalFunc = EqualFunc())
		: mHashFunc(hashFunc),
		mEqualFunc(equalFunc),
		mMaxLoadFactor(0.0)
	{
		startBucketCount = std::minmax(startBucketCount, (size_t)8).second;
		mLogStartBucketCount = (uint8_t)internal::UIntMath<size_t>::Log2(startBucketCount - 1) + 1;
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
		return (size_t)mLogStartBucketCount;
	}

	template<typename KeyArg>
	size_t GetHashCode(const KeyArg& key) const
	{
		MOMO_STATIC_ASSERT((std::is_same<Key, KeyArg>::value) || IsValidKeyArg<KeyArg>::value);
		return mHashFunc(key);
	}

	template<typename KeyArg1, typename KeyArg2>
	bool IsEqual(const KeyArg1& key1, const KeyArg2& key2) const
	{
		MOMO_STATIC_ASSERT((std::is_same<Key, KeyArg1>::value) || IsValidKeyArg<KeyArg1>::value);
		MOMO_STATIC_ASSERT((std::is_same<Key, KeyArg2>::value));
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

/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/HashTraits.h

  namespace momo:
    struct IsFastNothrowHashable
    struct HashCoder
    struct HashBucketDefault
    struct HashBucketOpenDefault
    class HashTraits
    class HashTraitsOpen
    class HashTraitsVar
    class HashTraitsStd

\**********************************************************/

#pragma once

#include "details/HashBucketLimP4.h"
#include "details/HashBucketOpen2N2.h"

#ifdef MOMO_INCLUDE_OLD_HASH_BUCKETS
#include "details/HashBucketLim4.h"
#include "details/HashBucketLimP.h"
#include "details/HashBucketLimP1.h"
#include "details/HashBucketUnlimP.h"
#include "details/HashBucketOneIA.h"
#include "details/HashBucketOneI1.h"
#include "details/HashBucketOneI.h"
#include "details/HashBucketOpen1.h"
#include "details/HashBucketOpen2N.h"
#include "details/HashBucketOpenN.h"
#include "details/HashBucketOpenN1.h"
#endif

namespace momo
{

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
		MOMO_NOEXCEPT_IF(IsFastNothrowHashable<Key>::value)
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
	using IsValidKeyArg = std::false_type;

	static const bool isFastNothrowHashable = IsFastNothrowHashable<Key>::value;

public:
	explicit HashTraits() MOMO_NOEXCEPT
	{
	}

	size_t CalcCapacity(size_t bucketCount) const MOMO_NOEXCEPT
	{
		return HashBucket::CalcCapacity(bucketCount);
	}

	size_t GetBucketCountShift(size_t bucketCount) const MOMO_NOEXCEPT
	{
		return HashBucket::GetBucketCountShift(bucketCount);
	}

	size_t GetLogStartBucketCount() const MOMO_NOEXCEPT
	{
		return HashBucket::logStartBucketCount;
	}

	size_t GetHashCode(const Key& key) const
	{
		//MOMO_STATIC_ASSERT(std::is_empty<HashCoder<Key>>::value);
		return HashCoder<Key>()(key);
	}

	bool IsEqual(const Key& key1, const Key& key2) const
	{
		return key1 == key2;
	}
};

template<typename TKey>
using HashTraitsOpen = HashTraits<TKey, HashBucketOpenDefault>;

template<typename TKey,
	typename THashBucket = HashBucketDefault>
class HashTraitsVar
{
public:
	typedef TKey Key;
	typedef THashBucket HashBucket;

	typedef std::function<size_t(size_t)> CalcCapacityFunc;
	typedef std::function<size_t(size_t)> GetBucketCountShiftFunc;

	template<typename KeyArg>
	using IsValidKeyArg = std::false_type;

	static const bool isFastNothrowHashable = IsFastNothrowHashable<Key>::value;

public:
	explicit HashTraitsVar(const CalcCapacityFunc& calcCapacityFunc = HashBucket::CalcCapacity,
		GetBucketCountShiftFunc getBucketCountShiftFunc = HashBucket::GetBucketCountShift,
		size_t logStartBucketCount = HashBucket::logStartBucketCount)
		: mCalcCapacityFunc(calcCapacityFunc),
		mGetBucketCountShiftFunc(getBucketCountShiftFunc),
		mLogStartBucketCount(logStartBucketCount)
	{
	}

	explicit HashTraitsVar(float maxLoadFactor,
		GetBucketCountShiftFunc getBucketCountShiftFunc = HashBucket::GetBucketCountShift,
		size_t logStartBucketCount = HashBucket::logStartBucketCount)
		: mGetBucketCountShiftFunc(getBucketCountShiftFunc),
		mLogStartBucketCount(logStartBucketCount)
	{
		HashBucket::CheckMaxLoadFactor(maxLoadFactor);
		mCalcCapacityFunc = [maxLoadFactor] (size_t bucketCount)
			{ return static_cast<size_t>(static_cast<float>(bucketCount) * maxLoadFactor); };
	}

	size_t CalcCapacity(size_t bucketCount) const MOMO_NOEXCEPT
	{
		return mCalcCapacityFunc(bucketCount);
	}

	size_t GetBucketCountShift(size_t bucketCount) const MOMO_NOEXCEPT
	{
		return mGetBucketCountShiftFunc(bucketCount);
	}

	size_t GetLogStartBucketCount() const MOMO_NOEXCEPT
	{
		return mLogStartBucketCount;
	}

	size_t GetHashCode(const Key& key) const
	{
		//MOMO_STATIC_ASSERT(std::is_empty<HashCoder<Key>>::value);
		return HashCoder<Key>()(key);
	}

	bool IsEqual(const Key& key1, const Key& key2) const
	{
		return key1 == key2;
	}

private:
	CalcCapacityFunc mCalcCapacityFunc;
	GetBucketCountShiftFunc mGetBucketCountShiftFunc;
	size_t mLogStartBucketCount;
};

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
	using IsValidKeyArg = std::false_type;

	static const bool isFastNothrowHashable = IsFastNothrowHashable<Key>::value
		&& (std::is_same<HashFunc, HashCoder<Key>>::value
		|| std::is_same<HashFunc, std::hash<Key>>::value);

public:
	explicit HashTraitsStd(size_t startBucketCount = (size_t)1 << HashBucket::logStartBucketCount,
		const HashFunc& hashFunc = HashFunc(),
		const EqualFunc& equalFunc = EqualFunc())
		: mHashFunc(hashFunc),
		mEqualFunc(equalFunc)
	{
		startBucketCount = std::minmax(startBucketCount, (size_t)8).second;
		mLogStartBucketCount = (uint8_t)internal::UIntMath<size_t>::Log2(startBucketCount - 1) + 1;
		static const size_t testBucketCount = 1 << 16;
		size_t testCapacity = HashBucket::CalcCapacity(testBucketCount);
		mMaxLoadFactor = static_cast<float>(testCapacity) / static_cast<float>(testBucketCount);
		HashBucket::CheckMaxLoadFactor(mMaxLoadFactor);
	}

	HashTraitsStd(const HashTraitsStd& hashTraits, float maxLoadFactor)
		: mHashFunc(hashTraits.mHashFunc),
		mEqualFunc(hashTraits.mEqualFunc),
		mLogStartBucketCount(hashTraits.mLogStartBucketCount),
		mMaxLoadFactor(maxLoadFactor)
	{
		HashBucket::CheckMaxLoadFactor(mMaxLoadFactor);
	}

	size_t CalcCapacity(size_t bucketCount) const MOMO_NOEXCEPT
	{
		return static_cast<size_t>(static_cast<float>(bucketCount) * mMaxLoadFactor);
	}

	size_t GetBucketCountShift(size_t bucketCount) const MOMO_NOEXCEPT
	{
		return HashBucket::GetBucketCountShift(bucketCount);
	}

	size_t GetLogStartBucketCount() const MOMO_NOEXCEPT
	{
		return (size_t)mLogStartBucketCount;
	}

	size_t GetHashCode(const Key& key) const
	{
		return mHashFunc(key);
	}

	bool IsEqual(const Key& key1, const Key& key2) const
	{
		return mEqualFunc(key1, key2);
	}

	const HashFunc& GetHashFunc() const MOMO_NOEXCEPT
	{
		return mHashFunc;
	}

	const EqualFunc& GetEqualFunc() const MOMO_NOEXCEPT
	{
		return mEqualFunc;
	}

	float GetMaxLoadFactor() const MOMO_NOEXCEPT
	{
		return mMaxLoadFactor;
	}

private:
	HashFunc mHashFunc;
	EqualFunc mEqualFunc;
	uint8_t mLogStartBucketCount;
	float mMaxLoadFactor;
};

} // namespace momo

/**********************************************************\

  momo/HashTraits.h

  namespace momo:
    struct HashBucketDefault
    class HashTraits
    class HashTraitsStd

\**********************************************************/

#pragma once

#include "HashBuckets/BucketLimP1.h"
#include "HashBuckets/BucketLimP.h"
#include "HashBuckets/BucketUnlimP.h"
#include "HashBuckets/BucketOneI1.h"
//#include "HashBuckets/BucketLim4.h"

namespace momo
{

typedef MOMO_DEFAULT_HASH_BUCKET HashBucketDefault;

template<typename TKey,
	typename THashBucket = HashBucketDefault>
class HashTraits
{
public:
	typedef TKey Key;
	typedef THashBucket HashBucket;
	typedef std::function<size_t(size_t)> CalcCapacityFunc;
	typedef std::function<size_t(size_t)> GetBucketCountShiftFunc;

public:
	explicit HashTraits(const CalcCapacityFunc& calcCapacityFunc = HashBucket::CalcCapacity,
		GetBucketCountShiftFunc getBucketCountShiftFunc = HashBucket::GetBucketCountShift,
		size_t logStartBucketCount = HashBucket::logStartBucketCount)
		: mCalcCapacityFunc(calcCapacityFunc),
		mGetBucketCountShiftFunc(getBucketCountShiftFunc),
		mLogStartBucketCount(logStartBucketCount)
	{
	}

	explicit HashTraits(float maxLoadFactor,
		GetBucketCountShiftFunc getBucketCountShiftFunc = HashBucket::GetBucketCountShift,
		size_t logStartBucketCount = HashBucket::logStartBucketCount)
		: mGetBucketCountShiftFunc(getBucketCountShiftFunc),
		mLogStartBucketCount(logStartBucketCount)
	{
		HashBucket::CheckMaxLoadFactor(maxLoadFactor);
		mCalcCapacityFunc = [maxLoadFactor] (size_t bucketCount)
			{ return (size_t)((float)bucketCount * maxLoadFactor); };
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
		return std::hash<Key>()(key);
	}

	bool IsEqual(const Key& key1, const Key& key2) const
	{
		return key1 == key2;
	}

	size_t GetBucketIndex(size_t hashCode, size_t bucketCount, size_t probe) const MOMO_NOEXCEPT
	{
		return HashBucket::GetBucketIndex(hashCode, bucketCount, probe);
	}

private:
	CalcCapacityFunc mCalcCapacityFunc;
	GetBucketCountShiftFunc mGetBucketCountShiftFunc;
	size_t mLogStartBucketCount;
};

template<typename TKey,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename THashBucket = HashBucketDefault>
class HashTraitsStd
{
public:
	typedef TKey Key;
	typedef THashFunc HashFunc;
	typedef TEqualFunc EqualFunc;
	typedef THashBucket HashBucket;

public:
	explicit HashTraitsStd(size_t startBucketCount = (size_t)1 << HashBucket::logStartBucketCount,
		const HashFunc& hashFunc = HashFunc(),
		const EqualFunc& equalFunc = EqualFunc())
		: mHashFunc(hashFunc),
		mEqualFunc(equalFunc)
	{
		startBucketCount = std::minmax(startBucketCount, (size_t)8).second;
		mLogStartBucketCount = internal::UIntMath<size_t>::Log2(startBucketCount - 1) + 1;
		startBucketCount = (size_t)1 << mLogStartBucketCount;
		size_t startCapacity = HashBucket::CalcCapacity(startBucketCount);
		mMaxLoadFactor = (float)startCapacity / (float)startBucketCount;
		HashBucket::CheckMaxLoadFactor(mMaxLoadFactor);
	}

	HashTraitsStd(const HashFunc& hashFunc, const EqualFunc& equalFunc,
		size_t logStartBucketCount, float maxLoadFactor)
		: mHashFunc(hashFunc),
		mEqualFunc(equalFunc),
		mLogStartBucketCount(logStartBucketCount)
	{
		HashBucket::CheckMaxLoadFactor(maxLoadFactor);
		mMaxLoadFactor = maxLoadFactor;
	}

	size_t CalcCapacity(size_t bucketCount) const MOMO_NOEXCEPT
	{
		return (size_t)((float)bucketCount * mMaxLoadFactor);
	}

	size_t GetBucketCountShift(size_t bucketCount) const MOMO_NOEXCEPT
	{
		return HashBucket::GetBucketCountShift(bucketCount);
	}

	size_t GetLogStartBucketCount() const MOMO_NOEXCEPT
	{
		return mLogStartBucketCount;
	}

	size_t GetHashCode(const Key& key) const
	{
		return mHashFunc(key);
	}

	bool IsEqual(const Key& key1, const Key& key2) const
	{
		return mEqualFunc(key1, key2);
	}

	size_t GetBucketIndex(size_t hashCode, size_t bucketCount, size_t probe) const MOMO_NOEXCEPT
	{
		return HashBucket::GetBucketIndex(hashCode, bucketCount, probe);
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
	size_t mLogStartBucketCount;
	float mMaxLoadFactor;
};

} // namespace momo

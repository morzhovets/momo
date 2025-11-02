/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
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

#ifndef MOMO_INCLUDE_GUARD_HASH_TRAITS
#define MOMO_INCLUDE_GUARD_HASH_TRAITS

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

namespace internal
{
#ifdef MOMO_USE_HASH_TRAITS_STRING_SPECIALIZATION
	template<typename TString,
		typename TChar = typename TString::value_type,
		typename TCharTraits = typename TString::traits_type,
		typename TStringView = std::basic_string_view<TChar, TCharTraits>,
		typename = EnableIf<std::is_convertible_v<const TString&, TStringView>
			&& !std::is_same_v<TString, TStringView>>>
	struct HashTraitsStringViewSelector
	{
		typedef TString String;
		typedef TStringView StringView;
	};
#endif

	template<typename Hasher, typename EqualComparer,
		typename = void>
	struct HashTraitsStdIsValidKeyArg : public std::false_type
	{
	};

	template<typename Hasher, typename EqualComparer>
	struct HashTraitsStdIsValidKeyArg<Hasher, EqualComparer,
		Void<typename Hasher::is_transparent, typename EqualComparer::is_transparent>>
		: public std::true_type
	{
	};

	template<typename HashTraits, typename ItemTraits,
		typename = void>
	struct HashTraitsBucketSelector
	{
		MOMO_DEPRECATED typedef typename HashTraits::HashBucket
			::template Bucket<ItemTraits, !HashTraits::isFastNothrowHashable> Bucket;
	};

	template<typename HashTraits, typename ItemTraits>
	struct HashTraitsBucketSelector<HashTraits, ItemTraits,
		Void<typename HashTraits::template Bucket<ItemTraits>>>
	{
		typedef typename HashTraits::template Bucket<ItemTraits> Bucket;
	};
}

template<typename Key>
struct IsFastNothrowHashable : public internal::BoolConstant<MOMO_IS_FAST_NOTHROW_HASHABLE(Key)>
{
};

template<typename Key,
	typename Result = size_t>
struct HashCoder : private std::hash<Key>
{
	Result operator()(const Key& key) const
		//noexcept(noexcept(std::hash<Key>::operator()(key)))	// gcc
	{
		return static_cast<Result>(std::hash<Key>::operator()(key));
	}
};

#ifdef MOMO_HASH_CODER
template<typename Key>
struct HashCoder<Key, decltype(MOMO_HASH_CODER(std::declval<const Key&>()))>
{
	decltype(MOMO_HASH_CODER(std::declval<const Key&>())) operator()(const Key& key) const
		//noexcept(noexcept(MOMO_HASH_CODER(key)))
	{
		return MOMO_HASH_CODER(key);
	}
};
#endif

typedef MOMO_DEFAULT_HASH_BUCKET HashBucketDefault;

typedef MOMO_DEFAULT_HASH_BUCKET_OPEN HashBucketOpenDefault;

template<typename TKey,
	typename THashBucket = HashBucketDefault,
	typename TBaseKeyArg = TKey>
class HashTraits
{
public:
	typedef TKey Key;
	typedef THashBucket HashBucket;
	typedef TBaseKeyArg BaseKeyArg;

	static const bool isFastNothrowHashable = IsFastNothrowHashable<BaseKeyArg>::value;

	template<typename ItemTraits>
	using Bucket = typename HashBucket::template Bucket<ItemTraits, !isFastNothrowHashable>;

	template<typename KeyArg>
	using IsValidKeyArg = typename std::conditional<std::is_same<BaseKeyArg, Key>::value,
		std::false_type, std::is_convertible<const KeyArg&, const BaseKeyArg&>>::type;	//?

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
		return HashCoder<BaseKeyArg>()(static_cast<const BaseKeyArg&>(key));
	}

	template<typename KeyArg1, typename KeyArg2>
	bool IsEqual(const KeyArg1& key1, const KeyArg2& key2) const
	{
		return static_cast<const BaseKeyArg&>(key1) == static_cast<const BaseKeyArg&>(key2);
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
	typename THasher = HashCoder<TKey>,
	typename TEqualComparer = std::equal_to<TKey>,
	typename THashBucket = HashBucketDefault>
class HashTraitsStd
{
public:
	typedef TKey Key;
	typedef THasher Hasher;
	typedef TEqualComparer EqualComparer;
	typedef THashBucket HashBucket;

	static const bool isFastNothrowHashable = IsFastNothrowHashable<Key>::value
		&& (std::is_same<Hasher, HashCoder<Key>>::value
		|| std::is_same<Hasher, std::hash<Key>>::value);

	template<typename ItemTraits>
	using Bucket = typename HashBucket::template Bucket<ItemTraits, !isFastNothrowHashable>;

	template<typename KeyArg>
	using IsValidKeyArg = internal::HashTraitsStdIsValidKeyArg<Hasher, EqualComparer>;

#ifdef MOMO_USE_UNORDERED_HINT_ITERATORS
	static const bool useHintIterators = true;
#else
	static const bool useHintIterators = false;
#endif

public:
	explicit HashTraitsStd(size_t startBucketCount = size_t{1} << HashBucket::logStartBucketCount,
		const Hasher& hasher = Hasher(),
		const EqualComparer& equalComp = EqualComparer())
		: mHasher(hasher),
		mEqualComparer(equalComp),
		mMaxLoadFactor(0.0)
	{
		startBucketCount = internal::UIntMath<>::Max(startBucketCount, 8);
		mLogStartBucketCount = static_cast<uint8_t>(internal::UIntMath<>::Log2(startBucketCount - 1)) + 1;
	}

	HashTraitsStd(const HashTraitsStd& hashTraits, float maxLoadFactor)
		: mHasher(hashTraits.mHasher),
		mEqualComparer(hashTraits.mEqualComparer),
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
		return mHasher(key);
	}

	template<typename KeyArg1, typename KeyArg2>
	bool IsEqual(const KeyArg1& key1, const KeyArg2& key2) const
	{
		return mEqualComparer(key1, key2);
	}

	const Hasher& GetHasher() const noexcept
	{
		return mHasher;
	}

	const EqualComparer& GetEqualComparer() const noexcept
	{
		return mEqualComparer;
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
	Hasher mHasher;
	EqualComparer mEqualComparer;
	uint8_t mLogStartBucketCount;
	float mMaxLoadFactor;
};

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_HASH_TRAITS

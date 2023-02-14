/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/MergeTraits.h

  namespace momo:
    concept conceptMergeBloomFilter
    class MergeBloomFilter
    class MergeBloomFilterEmpty
    enum class MergeTraitsFunc
    concept conceptMergeTraits
    class MergeTraits
    class MergeTraitsHash

\**********************************************************/

#pragma once

#include "KeyUtility.h"
#include "MergeArray.h"

namespace momo
{

template<typename MergeBloomFilter>
concept conceptMergeBloomFilter =
	std::is_nothrow_destructible_v<MergeBloomFilter> &&
	std::is_default_constructible_v<MergeBloomFilter> &&
	std::is_nothrow_move_constructible_v<MergeBloomFilter> &&
	requires
	{
		typename std::bool_constant<MergeBloomFilter::isAlwaysEmpty>;
	};

template<size_t tLogMult = 3>
class MergeBloomFilter
{
public:
	static const size_t logMult = tLogMult;

	static const bool isAlwaysEmpty = false;

public:
	explicit MergeBloomFilter() noexcept
		: mData(nullptr)
	{
	}

	MergeBloomFilter(MergeBloomFilter&& filter) noexcept
		: mData(std::exchange(filter.mData, nullptr))
	{
	}

	MergeBloomFilter(const MergeBloomFilter&) = delete;

	~MergeBloomFilter() noexcept
	{
		MOMO_ASSERT(IsEmpty());
	}

	MergeBloomFilter& operator=(const MergeBloomFilter&) = delete;

	void Swap(MergeBloomFilter& filter) noexcept
	{
		std::swap(mData, filter.mData);
	}

	bool IsEmpty() const noexcept
	{
		return mData == nullptr;
	}

	template<conceptMemManager MemManager>
	void Init(MemManager& memManager, size_t logMaxCount)
	{
		MOMO_ASSERT(IsEmpty());
		size_t byteSize = pvGetByteSize(logMaxCount);
		mData = internal::MemManagerProxy<MemManager>::template Allocate<uint8_t>(
			memManager, byteSize);
		std::uninitialized_fill_n(mData, byteSize, uint8_t{0});
	}

	template<conceptMemManager MemManager>
	void Init(MemManager& memManager, size_t logMaxCount, const MergeBloomFilter& filter)
	{
		MOMO_ASSERT(IsEmpty());
		size_t byteSize = pvGetByteSize(logMaxCount);
		mData = internal::MemManagerProxy<MemManager>::template Allocate<uint8_t>(
			memManager, byteSize);
		std::uninitialized_copy_n(filter.mData, byteSize, mData);
	}

	template<conceptMemManager MemManager>
	void Clear(MemManager& memManager, size_t logMaxCount)
	{
		MOMO_ASSERT(!IsEmpty());
		internal::MemManagerProxy<MemManager>::Deallocate(memManager,
			mData, pvGetByteSize(logMaxCount));
		mData = nullptr;
	}

	void Set(size_t hashCode, size_t logMaxCount) noexcept
	{
		MOMO_ASSERT(!IsEmpty());
		internal::UIntMath<uint8_t>::SetBit(mData, pvGetBitIndex1(hashCode, logMaxCount));
		internal::UIntMath<uint8_t>::SetBit(mData, pvGetBitIndex2(hashCode, logMaxCount));
	}

	bool Test(size_t hashCode, size_t logMaxCount) const noexcept
	{
		MOMO_ASSERT(!IsEmpty());
		return internal::UIntMath<uint8_t>::GetBit(mData, pvGetBitIndex1(hashCode, logMaxCount))
			&& internal::UIntMath<uint8_t>::GetBit(mData, pvGetBitIndex2(hashCode, logMaxCount));
	}

private:
	static size_t pvGetByteSize(size_t logMaxCount) noexcept
	{
		return size_t{1} << (std::minmax(logMaxCount + logMult, size_t{3}).second - 3);
	}

	static size_t pvGetBitIndex1(size_t hashCode, size_t logMaxCount) noexcept
	{
		return hashCode & ((size_t{1} << (logMaxCount + logMult)) - 1);
	}

	static size_t pvGetBitIndex2(size_t hashCode, size_t logMaxCount) noexcept
	{
		return hashCode >> (8 * sizeof(size_t) - (logMaxCount + logMult));
	}

private:
	uint8_t* mData;
};

class MergeBloomFilterEmpty
{
public:
	static const bool isAlwaysEmpty = true;

public:
	explicit MergeBloomFilterEmpty() noexcept = default;

	MergeBloomFilterEmpty(MergeBloomFilterEmpty&&) noexcept = default;

	MergeBloomFilterEmpty(const MergeBloomFilterEmpty&) = delete;

	~MergeBloomFilterEmpty() noexcept = default;

	MergeBloomFilterEmpty& operator=(const MergeBloomFilterEmpty&) = delete;

	void Swap(MergeBloomFilterEmpty& /*filter*/) noexcept
	{
	}

	template<conceptMemManager MemManager>
	void Init(MemManager& /*memManager*/, size_t /*logMaxCount*/,
		const MergeBloomFilterEmpty& /*filter*/)
	{
	}
};

enum class MergeTraitsFunc
{
	hash = 0,
	lessNothrow = 1,
	lessThrow = 2,
};

template<typename MergeTraits, typename Key>
concept conceptMergeTraits =
	std::copy_constructible<MergeTraits> &&
	conceptMergeBloomFilter<typename MergeTraits::BloomFilter> &&
	requires (const MergeTraits& mergeTraits, const Key& key)
	{
		typename MergeTraits::MergeArraySettings;
		typename std::integral_constant<MergeTraitsFunc, MergeTraits::func>;
		//{ mergeTraits.GetHashCode(key) } -> std::same_as<size_t>;
		//{ mergeTraits.IsLess(key, key) } -> std::same_as<bool>;
		{ mergeTraits.IsEqual(key, key) } -> std::same_as<bool>;
	};

template<conceptObject TKey,
	MergeTraitsFunc tFunc = noexcept(std::declval<const TKey&>() < std::declval<const TKey&>())
		? MergeTraitsFunc::lessNothrow : MergeTraitsFunc::lessThrow,
	typename TMergeArraySettings = MergeArraySettings<3>,
	typename TBloomFilter = MergeBloomFilterEmpty>
class MergeTraits
{
public:
	typedef TKey Key;
	typedef TMergeArraySettings MergeArraySettings;
	typedef TBloomFilter BloomFilter;

	static const MergeTraitsFunc func = tFunc;

public:
	explicit MergeTraits() noexcept = default;

	size_t GetHashCode(const Key& key) const
		requires requires { { HashCoder<Key>()(key) } -> std::convertible_to<size_t>; }
	{
		return HashCoder<Key>()(key);
	}

	bool IsLess(const Key& key1, const Key& key2) const noexcept(func == MergeTraitsFunc::lessNothrow)
		requires requires { { key1 < key2 } -> std::convertible_to<bool>; }
	{
		return std::less<>()(key1, key2);
	}

	bool IsEqual(const Key& key1, const Key& key2) const
		requires requires { { key1 == key2 } -> std::convertible_to<bool>; }
	{
		return key1 == key2;
	}
};

template<conceptObject TKey>
using MergeTraitsHash = MergeTraits<TKey, MergeTraitsFunc::hash>;

} // namespace momo

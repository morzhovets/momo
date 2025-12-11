/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  test/sources/LibcxxMergeSetTester.h

\**********************************************************/

#pragma once

#include "LibcxxTester.h"

#include "../../include/momo/MergeSet.h"
#include "../../include/momo/stdish/unordered_set.h"

namespace
{

using namespace libcxx_insert_range_maps_sets;
using namespace libcxx_from_range_unord;

namespace libcxx_merge_set
{

template<typename TKey, typename THasher, typename TEqualComparer>
class LibcxxMergeTraits
	: public momo::HashTraitsStd<TKey, THasher, TEqualComparer>
{
private:
	typedef momo::HashTraitsStd<TKey, THasher, TEqualComparer> HashTraitsStd;

public:
	using typename HashTraitsStd::Key;

	static const bool useHintIterators = false;

	typedef momo::MergeBloomFilterEmpty BloomFilter;

	static const size_t logInitialItemCount = 0;

#ifdef LIBCXX_TEST_MERGE_HASH
	static const momo::MergeTraitsFunc func = momo::MergeTraitsFunc::hash;
#else
	static const momo::MergeTraitsFunc func = momo::MergeTraitsFunc::lessNothrow;
#endif

public:
	using HashTraitsStd::HashTraitsStd;

#ifndef LIBCXX_TEST_MERGE_HASH
	template<typename KeyArg>
	bool IsLess(const Key& key1, const KeyArg& key2) const
	{
		if constexpr (requires { key1 < key2; })
			return std::less<>()(key1, key2);
		else
			return HashTraitsStd::GetHashCode(key1) < HashTraitsStd::GetHashCode(key2);
	}
#endif
};

class LibcxxMergeSetSettings : public momo::MergeSetSettings
{
public:
	static const momo::CheckMode checkMode = momo::CheckMode::exception;
	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION;
};

template<typename MergeSet>
class LibcxxMergeSet : public MergeSet
{
public:
	typedef typename MergeSet::MergeTraits HashTraits;

	typedef momo::internal::ArrayBounds<const typename MergeSet::Item*> ConstBucketBounds;

	static const size_t bucketMaxItemCount = SIZE_MAX;

public:
	using MergeSet::MergeSet;

	const HashTraits& GetHashTraits() const noexcept
	{
		return MergeSet::GetMergeTraits();
	}

	size_t GetBucketCount() const noexcept
	{
		return SIZE_MAX;
	}

	void Reserve(size_t /*capacity*/)
	{
	}

	void MergeFrom(LibcxxMergeSet& srcSet)
	{
		assert(MergeSet::IsEmpty());
		MergeSet::Insert(srcSet.GetBegin(), srcSet.GetEnd());
		srcSet.Clear();
	}
};

namespace std
{
	using namespace ::std;

	template<typename TKey,
		typename THasher = std::hash<TKey>,
		typename TEqualComparer = std::equal_to<TKey>,
		typename TAllocator = std::allocator<TKey>>
	using unordered_set = momo::stdish::unordered_set_adaptor<LibcxxMergeSet<momo::MergeSetCore<
		momo::MergeSetItemTraits<TKey, momo::MemManagerStd<TAllocator>>,
		LibcxxMergeTraits<TKey, THasher, TEqualComparer>,
		LibcxxMergeSetSettings>>>;
}

#define LIBCXX_TEST_FAILURE
#define LIBCXX_TEST_MERGE_SET
#define LIBCXX_TEST_PREFIX "libcxx_merge_set" LIBCXX_TEST_PREFIX_TAIL
#include "libcxx/UnorderedSetTests.h"
#undef LIBCXX_TEST_PREFIX
#undef LIBCXX_TEST_MERGE_SET
#undef LIBCXX_TEST_FAILURE

} // namespace libcxx_merge_set

} // namespace
